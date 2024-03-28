#include "network.h"

#include "game/tetrisgame.h"
#include "protobufmessagequeue.h"
#include "util.h"
#include "protocol.h"
#include "random.h"

#include <message.pb.h>

#include <net/client.h>

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <helper.h>

#include <concurrencpp/concurrencpp.h>

namespace tp = tetris_protocol;

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

		void send(const tp::Wrapper& wrapper) {
			ProtobufMessage message;
			client_->acquire(message);
			message.setBuffer(wrapper_);
			client_->send(std::move(message));
		}

		void update() {
			manualExecutor_->loop_once();
			cv_.notify_all();
		}

		conc::result<void> stepOnce() {
			co_await nextMessage();
			if (wrapper_.has_failed_to_connect()) {
				connected_ = false;
				connected(connected_);
				co_return;
			} else if (wrapper_.has_game_looby()) {
				handleGameLooby(wrapper_.game_looby());
				connected_ = true;
				connected(connected_);
			} else {
				co_return;
			}

			while (true) {
				co_await nextMessage();
				if (wrapper_.has_game_looby()) {
					handleGameCommand(wrapper_.game_command());
				}
				if (wrapper_.has_connections()) {
					handleConnections(wrapper_.connections());
				}
				if (wrapper_.has_create_server_game()) {
					handleConnections(wrapper_.connections());
					break;
				}
			}
			
			while (true) {
				co_await nextMessage();
				if (wrapper_.has_game_command()) {
					handleGameCommand(wrapper_.game_command());
				}
				if (wrapper_.has_game_restart()) {
					handleGameRestart(wrapper_.game_restart());
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
					wrapper_.Clear();
					bool valid = wrapper_.ParseFromArray(message.getBodyData(), message.getBodySize());
					client_->release(std::move(message));
				}
				return valid;
			});
			co_return;
		}

		void handleGameRestart(const tp::GameRestart& gameRestart) {
			auto current = static_cast<tetris::BlockType>(gameRestart.current());
			auto next = static_cast<tetris::BlockType>(gameRestart.next());
			tetrisGame_->restartGame(current, next);
		}

		void handleGameCommand(const tp::GameCommand& gameCommand) {
			spdlog::info("[Network] Paused: {}", gameCommand.pause() ? "true" : "false");
			tetrisGame_->pause();
		}

		void handleGameLooby(const tp::GameLooby& gameLooby) {
			connected_ = true;
		}

		void handleConnections(const tp::Connections& connections) {
			for (const auto& uuid : connections.uuids()) {
				spdlog::info("[Network] Connected uuid: {}", uuid);
			}
		}

		void setPlayerSlot(const game::PlayerSlot& playerSlot, int index) {
			wrapper_.Clear();
			auto tpGameLooby = wrapper_.mutable_game_looby();

			if (index >= 0 && index < playerSlots_.size()) {
				playerSlots_[index] = playerSlot;
				playerSlotUpdate(playerSlot, index);
				
				for (const auto& slot : playerSlots_) {
					auto tpSlot = tpGameLooby->add_slots();
					toTpSlot(slot, *tpSlot);
				}
				send(wrapper_);
			}
		}

		void createGameLooby(const std::string& uuid) {
			connections_.clear();

			wrapper_.Clear();
			auto gameLooby = wrapper_.mutable_game_looby();
			//connectToGame->set_server_uuid(uuid);
			//connectToGame->set_uuid(client_->getUuid());
			send(wrapper_);
		}

		void connectToGame(const std::string& uuid) {
			connections_.clear();
			
			wrapper_.Clear();
			auto connectToGame = wrapper_.mutable_connect_to_game();
			connectToGame->set_server_uuid(uuid);
			connectToGame->set_uuid(client_->getUuid());
			send(wrapper_);
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
			wrapper_.Clear();
			auto createServerGame = wrapper_.mutable_create_server_game();
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
			send(wrapper_);
		}

		const std::string& getServerId() const {
			return client_->getUuid();
		}

		void handleGameRestart(game::GameRestart gameRestart) {
			wrapper_.Clear();
			auto tpGameRestart = wrapper_.mutable_game_restart();
			tpGameRestart->set_current(static_cast<tp::BlockType>(gameRestart.current));
			tpGameRestart->set_next(static_cast<tp::BlockType>(gameRestart.next));
			send(wrapper_);
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
			wrapper_.Clear();
			auto boardMove = wrapper_.mutable_board_move();
			boardMove->set_uuid(player.getUuid());
			boardMove->set_move(static_cast<tp::Move>(updateMove.move));
			send(wrapper_);
		}

		void handlePlayerBoardUpdate(const game::Player& player, const game::UpdateNextBlock& updateNextBlock) {
			//spdlog::info("[Network] handle UpdateNextBlock: {}", static_cast<char>(updateNextBlock.next));
			wrapper_.Clear();
			auto nextBlock = wrapper_.mutable_next_block();
			nextBlock->set_uuid(player.getUuid());
			nextBlock->set_next(static_cast<tp::BlockType>(updateNextBlock.next));
			send(wrapper_);
		}

		void handleBoardEvent(const game::Player& player, tetris::BoardEvent boardEvent, int nbr) {
			spdlog::info("[Network] handle UpdateMove: {}, {}", static_cast<int>(boardEvent), nbr);
		}

		void sendPause(bool pause) {
			wrapper_.Clear();
			wrapper_.mutable_game_command()->set_pause(pause);
			send(wrapper_);
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
		
		tp::Wrapper wrapper_;
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
