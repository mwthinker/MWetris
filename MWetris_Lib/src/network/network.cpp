#include "network.h"

#include "game/tetrisgame.h"
#include "protobufmessagequeue.h"
#include "util.h"
#include "protocol.h"
#include "random.h"

#include <shared.pb.h>
#include <client_to_server.pb.h>
#include <server_to_client.pb.h>

#include <net/client.h>

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <helper.h>

#include <concurrencpp/concurrencpp.h>

namespace conc = concurrencpp;

namespace mwetris::network {

	namespace {

		constexpr std::string_view ServerIp = "127.0.0.1";
		constexpr int Port = 59412;

		std::vector<game::Human> extractHumans(const std::vector<game::PlayerSlot>& playerSlots) {
			std::vector<game::Human> humans;
			for (const auto& playerSlot : playerSlots) {
				if (auto human = std::get_if<game::Human>(&playerSlot); human) {
					humans.push_back(game::Human{
						.name = human->name,
						.device = human->device
					});
				}
			}
			return humans;
		}

		std::vector<game::Ai> extractAis(const std::vector<game::PlayerSlot>& playerSlots) {
			std::vector<game::Ai> ais;
			for (const auto& playerSlot : playerSlots) {
				if (auto ai = std::get_if<game::Ai>(&playerSlot); ai) {
					ais.push_back(game::Ai{
						.name = ai->name,
						.ai = ai->ai}
					);
				}
			}
			return ais;
		}

		tp::SlotType playerSlotToTpSlotType(const game::PlayerSlot& playerSlot) {
			tp::SlotType slotType = tp::SlotType::UNSPECIFIED_SLOT_TYPE;
			std::visit([&](auto&& slot) mutable {
				using T = std::decay_t<decltype(slot)>;
				if constexpr (std::is_same_v<T, game::Human>) {
					slotType = tp::SlotType::HUMAN;
				} else if constexpr (std::is_same_v<T, game::Ai>) {
					slotType = tp::SlotType::AI;
				} else if constexpr (std::is_same_v<T, game::Remote>) {
					slotType = tp::SlotType::REMOTE;
				} else if constexpr (std::is_same_v<T, game::OpenSlot>) {
					slotType = tp::SlotType::OPEN_SLOT;
				} else if constexpr (std::is_same_v<T, game::ClosedSlot>) {
					slotType = tp::SlotType::CLOSED_SLOT;
				} else {
					static_assert(always_false_v<T>, "non-exhaustive visitor!");
				}
			}, playerSlot);
			return slotType;
		}

	}

	class Network::Impl {
	public:
		mw::PublicSignal<Network::Impl, game::PlayerSlot, int> playerSlotUpdate;
		mw::PublicSignal<Network::Impl, bool> connected;

		Impl(std::shared_ptr<Client> client, std::shared_ptr<game::TetrisGame> tetrisGame)
			: client_{client}
			, tetrisGame_{tetrisGame} {
			
			for (int i = 0; i < 4; ++i) {
				playerSlots_.push_back(game::OpenSlot{});
			}

			manualExecutor_ = runtime_.make_executor<conc::manual_executor>();
			manualExecutor_->post([this]() {
				stepOnce();
			});
		}

		~Impl() {
		}

		void send(const tp_c2s::Wrapper& wrapper) {
			ProtobufMessage message;
			client_->acquire(message);
			message.setBuffer(wrapper);
			client_->send(std::move(message));
		}

		void update() {
			manualExecutor_->loop_once();
			cv_.notify_all();
		}

		conc::result<void> stepOnce() {
			// Connect to server
			co_await nextMessage();
			if (wrapperFromServer_.has_failed_to_connect()) {
				connected_ = false;
				connected(connected_);
				co_return;
			} else if (wrapperFromServer_.has_game_looby()) {
				handleGameLooby(wrapperFromServer_.game_looby());
				connected_ = true;
				connected(connected_);
			} else {
				co_return;
			}

			// Wait for game to start
			while (true) {
				co_await nextMessage();
				if (wrapperFromServer_.has_game_looby()) {
					handleGameCommand(wrapperFromServer_.game_command());
				}
				if (wrapperFromServer_.has_connections()) {
					handleConnections(wrapperFromServer_.connections());
				}
				if (wrapperFromServer_.has_create_server_game()) {
					handleConnections(wrapperFromServer_.connections());
					break;
				}
			}

			// Game loop
			while (true) {
				co_await nextMessage();
				if (wrapperFromServer_.has_game_command()) {
					handleGameCommand(wrapperFromServer_.game_command());
				}
				if (wrapperFromServer_.has_game_restart()) {
					handleGameRestart(wrapperFromServer_.game_restart());
				}
			}
			co_return;
		}

		void handleConnection() {
		}

		conc::result<void> nextMessage() {
			auto guard = co_await lock_.lock(manualExecutor_);
			co_await cv_.await(manualExecutor_, guard, [this]() {
				ProtobufMessage message;
				bool valid = client_->receive(message);
				if (valid) {
					wrapperFromServer_.Clear();
					bool valid = wrapperFromServer_.ParseFromArray(message.getBodyData(), message.getBodySize());
					client_->release(std::move(message));
				}
				return valid;
			});
			co_return;
		}

		void handleGameRestart(const tp_s2c::GameRestart& gameRestart) {
			auto current = static_cast<tetris::BlockType>(gameRestart.current());
			auto next = static_cast<tetris::BlockType>(gameRestart.next());
			tetrisGame_->restartGame(current, next);
		}

		void handleGameCommand(const tp_s2c::GameCommand& gameCommand) {
			spdlog::info("[Network] Paused: {}", gameCommand.pause() ? "true" : "false");
			tetrisGame_->pause();
		}

		void handleGameLooby(const tp_s2c::GameLooby& gameLooby) {
			connected_ = true;
		}

		void handleConnections(const tp_s2c::Connections& connections) {
			for (const auto& uuid : connections.uuids()) {
				spdlog::info("[Network] Connected uuid: {}", uuid);
			}
		}

		void setPlayerSlot(const game::PlayerSlot& playerSlot, int index) {
			wrapperToServer_.Clear();
			auto tpGameLooby = wrapperToServer_.mutable_game_looby();

			if (index >= 0 && index < playerSlots_.size()) {
				playerSlots_[index] = playerSlot;
				playerSlotUpdate(playerSlot, index);

				for (const auto& slot : playerSlots_) {
					auto tpSlot = tpGameLooby->add_slots();
					toTpSlot(slot, *tpSlot);
				}
				send(wrapperToServer_);
			}
		}

		void createGameLooby(const std::string& uuid) {
			connections_.clear();

			wrapperToServer_.Clear();
			auto gameLooby = wrapperToServer_.mutable_game_looby();
			//connectToGame->set_server_uuid(uuid);
			//connectToGame->set_uuid(client_->getUuid());
			send(wrapperToServer_);
		}

		void connectToGame(const std::string& uuid) {
			connections_.clear();
			
			wrapperToServer_.Clear();
			auto connectToGame = wrapperToServer_.mutable_connect_to_game();
			connectToGame->set_server_uuid(uuid);
			connectToGame->set_uuid(client_->getUuid());
			send(wrapperToServer_);
		}

		void disconnect() {
			
		}

		bool createGame(std::unique_ptr<game::GameRules> gameRules, int w, int h) {
			connections_.clear();
			localPlayers_ = game::PlayerFactory{}.createPlayers(w, h, extractHumans(playerSlots_), extractAis(playerSlots_));
			
			auto current = tetris::randomBlockType();
			auto next = tetris::randomBlockType();

			for (auto& player : localPlayers_) {
				player->updateRestart(current, next); // Update before attaching the event handles, to avoid multiple calls.
				connections_ += player->addPlayerBoardUpdateCallback([this, &player](game::PlayerBoardEvent playerBoardEvent) {
					std::visit([&](auto&& event) {
						handlePlayerBoardUpdate(*player, event);
					}, playerBoardEvent);
				});
				connections_ += player->addEventCallback([this, &player](tetris::BoardEvent boardEvent, int nbr) {
					handleBoardEvent(*player, boardEvent, nbr);
				});
			}

			tetrisGame_->createGame(std::move(gameRules), w, h, localPlayers_, {});
			connections_ += tetrisGame_->gameRestartEvent.connect([this](game::GameRestart gameRestart) {
				handleGameRestart(gameRestart);
			});
			
			sendCreateServerGame(current, next);
			return true;
		}

		void sendCreateServerGame(tetris::BlockType current, tetris::BlockType next) {
			wrapperToServer_.Clear();
			auto createServerGame = wrapperToServer_.mutable_create_server_game();
			createServerGame->set_current(static_cast<tp::BlockType>(current));
			createServerGame->set_next(static_cast<tp::BlockType>(next));
			for (auto& player : localPlayers_) {
				auto tpPlayer = createServerGame->add_local_players();
				tpPlayer->set_ai(player->isAi());
				tpPlayer->set_level(1);
				tpPlayer->set_points(1);
				tpPlayer->set_name(player->getName());
				tpPlayer->set_uuid(player->getUuid());
			}
			send(wrapperToServer_);
		}

		const std::string& getServerId() const {
			return client_->getUuid();
		}

		void handleGameRestart(game::GameRestart gameRestart) {
			wrapperToServer_.Clear();
			auto tpGameRestart = wrapperToServer_.mutable_game_restart();
			tpGameRestart->set_current(static_cast<tp::BlockType>(gameRestart.current));
			tpGameRestart->set_next(static_cast<tp::BlockType>(gameRestart.next));
			send(wrapperToServer_);
		}

		void handlePlayerBoardUpdate(const game::Player& player, const game::UpdateRestart& updateRestart) {
			spdlog::info("[Network] handle UpdateRestart: current={}, next={}", static_cast<char>(updateRestart.current), static_cast<char>(updateRestart.next));
		}

		void handlePlayerBoardUpdate(const game::Player& player, const game::UpdatePlayerData& updatePlayerData) {
			spdlog::info("[Network] handle UpdatePlayerData");
		}

		void handlePlayerBoardUpdate(const game::Player& player, const game::ExternalRows& externalRows) {
			spdlog::info("[Network] handle ExternalRows");
		}

		void handlePlayerBoardUpdate(const game::Player& player, const game::UpdateMove& updateMove) {
			//spdlog::info("[Network] handle UpdateMove: {}", static_cast<int>(updateMove.move));
			wrapperToServer_.Clear();
			auto boardMove = wrapperToServer_.mutable_board_move();
			boardMove->set_uuid(player.getUuid());
			boardMove->set_move(static_cast<tp::Move>(updateMove.move));
			send(wrapperToServer_);
		}

		void handlePlayerBoardUpdate(const game::Player& player, const game::UpdateNextBlock& updateNextBlock) {
			//spdlog::info("[Network] handle UpdateNextBlock: {}", static_cast<char>(updateNextBlock.next));
			wrapperToServer_.Clear();
			auto nextBlock = wrapperToServer_.mutable_next_block();
			nextBlock->set_uuid(player.getUuid());
			nextBlock->set_next(static_cast<tp::BlockType>(updateNextBlock.next));
			send(wrapperToServer_);
		}

		void handleBoardEvent(const game::Player& player, tetris::BoardEvent boardEvent, int nbr) {
			spdlog::info("[Network] handle UpdateMove: {}, {}", static_cast<int>(boardEvent), nbr);
		}

		void sendPause(bool pause) {
			wrapperToServer_.Clear();
			wrapperToServer_.mutable_game_command()->set_pause(pause);
			send(wrapperToServer_);
		}

	private:
		conc::runtime runtime_;
		std::shared_ptr<conc::manual_executor> manualExecutor_;
		conc::async_lock lock_;
		conc::async_condition_variable cv_;

		bool connected_ = false;
		std::vector<game::PlayerSlot> playerSlots_;
		mw::signals::ScopedConnections connections_;

		std::vector<game::PlayerPtr> localPlayers_;
		
		tp_c2s::Wrapper wrapperToServer_;
		tp_s2c::Wrapper wrapperFromServer_;
		std::shared_ptr<Client> client_;
		std::shared_ptr<game::TetrisGame> tetrisGame_;
	};

	Network::Network(std::shared_ptr<Client> client, std::shared_ptr<game::TetrisGame> tetrisGame)
		: impl_{std::make_unique<Network::Impl>(std::move(client), std::move(tetrisGame))} {
	}

	const std::string& Network::getServerId() const {
		return impl_->getServerId();
	}

	Network::~Network() {
	}

	void Network::sendPause(bool pause) {
		impl_->sendPause(pause);
	}

	void Network::update() {
		impl_->update();
	}

	void Network::setPlayerSlot(const game::PlayerSlot& playerSlot, int slot) {
		impl_->setPlayerSlot(playerSlot, slot);
	}

	void Network::createGameLooby(const std::string& serverId) {
		impl_->createGameLooby(serverId);
	}

	void Network::connectToGame(const std::string& serverId) {
		impl_->connectToGame(serverId);
	}

	void Network::disconnect() {
		impl_->disconnect();
	}

	bool Network::createGame(std::unique_ptr<game::GameRules> gameRules, int w, int h) {
		return impl_->createGame(std::move(gameRules), w, h);
	}

	mw::signals::Connection Network::addPlayerSlotListener(std::function<void(game::PlayerSlot, int)> listener) {
		return impl_->playerSlotUpdate.connect(listener);
	}

	mw::signals::Connection Network::addConnectionListener(std::function<void(bool)> listener) {
		return impl_->connected.connect(listener);
	}

}
