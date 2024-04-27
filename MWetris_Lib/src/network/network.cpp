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

	}

	class Network::Impl {
	public:
		mw::PublicSignal<Network::Impl, game::PlayerSlot, int> playerSlotUpdate;
		mw::PublicSignal<Network::Impl, bool> connected;

		struct NetworkPlayer {
			game::PlayerPtr player;
			std::string uuid;
		};

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
			} else if (wrapperFromServer_.has_connected()) {
				handleConnected(wrapperFromServer_.connected());
			} else {
				co_return;
			}

			// Wait for game to start
			while (true) {
				co_await nextMessage();
				if (wrapperFromServer_.has_game_looby()) {
					handleGameLooby(wrapperFromServer_.game_looby());
				}
				if (wrapperFromServer_.has_connections()) {
					handleConnections(wrapperFromServer_.connections());
				}
				if (wrapperFromServer_.has_create_game()) {
					handleCreateGame(wrapperFromServer_.create_game());
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
			connected_ = true;
			connected(connected_);
		}

		conc::result<void> nextMessage() {
			auto guard = co_await lock_.lock(manualExecutor_);
			co_await cv_.await(manualExecutor_, guard, [this]() {
				ProtobufMessage message;
				bool valid = client_->receive(message);
				if (valid) {
					wrapperFromServer_.Clear();
					valid = wrapperFromServer_.ParseFromArray(message.getBodyData(), message.getBodySize());
				} else if (message.getSize() != 0) {
					spdlog::info("[Network] Invalid data");
				}
				client_->release(std::move(message));
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

		void handleConnected(const tp_s2c::Connected& connected) {
			uuid_ = connected.uuid();
			spdlog::info("[Network] connected: {}", "");
			connected_ = true;
		}

		void handleGameLooby(const tp_s2c::GameLooby& gameLooby) {
			playerSlots_.clear();
			int index = 0;
			for (const auto& tpSlot : gameLooby.slots()) {
				switch (tpSlot.slot_type()) {
					case tp_s2c::GameLooby_SlotType_REMOTE:
						if (tpSlot.client_uuid() == uuid_) {
							if (tpSlot.ai()) {
								playerSlots_.push_back(game::Ai{.name = tpSlot.name()});
							} else {
								playerSlots_.push_back(game::Human{.name = tpSlot.name()});
							}
						} else {
							//playerSlots_.push_back(game::Remote{.name = tpSlot.name(), .uuid = tpSlot.player_uuid()});
						}
						break;
					case tp_s2c::GameLooby_SlotType_OPEN_SLOT:
						playerSlots_.push_back(game::OpenSlot{});
						break;
					case tp_s2c::GameLooby_SlotType_CLOSED_SLOT:
						playerSlots_.push_back(game::ClosedSlot{});
						break;
					default:
						continue;
				}
				playerSlotUpdate(playerSlots_[index], index);
				++index;
			}
		}

		void handleConnections(const tp_s2c::Connections& connections) {
			for (const auto& uuid : connections.uuids()) {
				spdlog::info("[Network] Connected uuid: {}", uuid);
			}
		}

		void setPlayerSlot(const game::PlayerSlot& playerSlot, int index) {
			wrapperToServer_.Clear();
			auto tpGameLooby = wrapperToServer_.mutable_player_slot();
			tpGameLooby->set_index(index);
			if (auto human = std::get_if<game::Human>(&playerSlot); human) {
				tpGameLooby->set_slot_type(tp_c2s::PlayerSlot_SlotType_HUMAN);
				tpGameLooby->set_name(human->name);
				deviceBySlotIndex_[index] = human->device;
				send(wrapperToServer_);
			} else if (auto ai = std::get_if<game::Ai>(&playerSlot); ai) {
				tpGameLooby->set_slot_type(tp_c2s::PlayerSlot_SlotType_AI);
				tpGameLooby->set_name(ai->name);
				aiBySlotIndex_[index] = ai->ai;
				send(wrapperToServer_);
			} else if (std::get_if<game::OpenSlot>(&playerSlot)) {
				tpGameLooby->set_slot_type(tp_c2s::PlayerSlot_SlotType_OPEN_SLOT);
				send(wrapperToServer_);
			} else if (std::get_if<game::ClosedSlot>(&playerSlot)) {
				tpGameLooby->set_slot_type(tp_c2s::PlayerSlot_SlotType_CLOSED_SLOT);
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
			send(wrapperToServer_);
		}

		void disconnect() {
			connected_ = false;
		}

		void handleCreateGame(int width, int height, const tp_s2c::CreateGame_Player& tpPlayer, const game::PlayerSlot& playerSlot) {
			auto current = static_cast<tetris::BlockType>(tpPlayer.current());
			auto next = static_cast<tetris::BlockType>(tpPlayer.next());
			
			if (auto human = std::get_if<game::Human>(&playerSlot); human) {
				auto& networkPlayer = players_.emplace_back(
					NetworkPlayer{
						.player = game::PlayerFactory{}.createPlayer(width, height, *human),
						.uuid = tpPlayer.player_uuid()
					}
				);
				networkPlayer.player->updateRestart(current, next); // Update before attaching the event handles, to avoid multiple calls.
				connections_ += networkPlayer.player->addPlayerBoardUpdateCallback([this, networkPlayer](game::PlayerBoardEvent playerBoardEvent) {
					std::visit([&](auto&& event) {
						handlePlayerBoardUpdate(networkPlayer, event);
					}, playerBoardEvent);
				});
				connections_ += networkPlayer.player->addEventCallback([this, player = networkPlayer.player](tetris::BoardEvent boardEvent, int nbr) {
					handleBoardEvent(*player, boardEvent, nbr);
				});
			} else if (auto ai = std::get_if<game::Ai>(&playerSlot); ai) {
				auto& networkPlayer = players_.emplace_back(
					NetworkPlayer{
						.player = game::PlayerFactory{}.createPlayer(height, height, *ai),
						.uuid = tpPlayer.player_uuid()
					}
				);
				networkPlayer.player->updateRestart(current, next); // Update before attaching the event handles, to avoid multiple calls.
				connections_ += networkPlayer.player->addPlayerBoardUpdateCallback([this, networkPlayer](game::PlayerBoardEvent playerBoardEvent) {
					std::visit([&](auto&& event) {
						handlePlayerBoardUpdate(networkPlayer, event);
					}, playerBoardEvent);
				});
				connections_ += networkPlayer.player->addEventCallback([this, networkPlayer](tetris::BoardEvent boardEvent, int nbr) {
					handleBoardEvent(*networkPlayer.player, boardEvent, nbr);
				});
			}
		}

		void handleCreateGame(const tp_s2c::CreateGame& createGame) {
			connections_.clear();
			players_.clear();
			fillSlotsWithDevicesAndAis();

			const auto& tpPlayers = createGame.players();

			int index = 0;
			for (const auto& tpPlayer : createGame.players()) {
				if (index >= playerSlots_.size()) {
					// TODO! Handle error!
					break;
				}
				handleCreateGame(createGame.width(), createGame.height(), tpPlayer, playerSlots_[index]);
				++index;
			}

			std::vector<game::PlayerPtr> players;
			for (auto& player : players_) {
				players.push_back(player.player);
			}
			tetrisGame_->createGame(std::move(gameRules_), players);
			gameRules_ = nullptr;
			connections_ += tetrisGame_->gameRestartEvent.connect([this](game::GameRestart gameRestart) {
				handleGameRestart(gameRestart);
			});
		}

		void fillSlotsWithDevicesAndAis() {
			for (int i = 0; i < playerSlots_.size(); ++i) {
				if (auto human = std::get_if<game::Human>(&playerSlots_[i]); human) {
					human->device = deviceBySlotIndex_[i];
				} else if (auto ai = std::get_if<game::Ai>(&playerSlots_[i]); ai) {
					ai->ai = aiBySlotIndex_[i];
				}
			}
		}

		void startGame(std::unique_ptr<game::GameRules> gameRules, int w, int h) {
			gameRules_ = std::move(gameRules);
			wrapperToServer_.Clear();
			wrapperToServer_.mutable_start_game()->set_ready(true);
			send(wrapperToServer_);
		}

		const std::string& getServerId() const {
			return uuid_;
		}

		void handleGameRestart(game::GameRestart gameRestart) {
			wrapperToServer_.Clear();
			auto tpGameRestart = wrapperToServer_.mutable_game_restart();
			tpGameRestart->set_current(static_cast<tp::BlockType>(gameRestart.current));
			tpGameRestart->set_next(static_cast<tp::BlockType>(gameRestart.next));
			send(wrapperToServer_);
		}

		void handlePlayerBoardUpdate(const NetworkPlayer& player, const game::UpdateRestart& updateRestart) {
			spdlog::info("[Network] handle UpdateRestart: current={}, next={}", static_cast<char>(updateRestart.current), static_cast<char>(updateRestart.next));
		}

		void handlePlayerBoardUpdate(const NetworkPlayer& player, const game::UpdatePlayerData& updatePlayerData) {
			spdlog::info("[Network] handle UpdatePlayerData");
		}

		void handlePlayerBoardUpdate(const NetworkPlayer& player, const game::ExternalRows& externalRows) {
			spdlog::info("[Network] handle ExternalRows");
		}

		void handlePlayerBoardUpdate(const NetworkPlayer& player, const game::UpdateMove& updateMove) {
			//spdlog::info("[Network] handle UpdateMove: {}", static_cast<int>(updateMove.move));
			wrapperToServer_.Clear();
			auto boardMove = wrapperToServer_.mutable_board_move();
			//auto uuid = player.getUuid();
			//boardMove->set_player_uuid(player.getUuid());
			boardMove->set_move(static_cast<tp::Move>(updateMove.move));
			boardMove->set_player_uuid(player.uuid);
			send(wrapperToServer_);
		}

		void handlePlayerBoardUpdate(const NetworkPlayer& player, const game::UpdateNextBlock& updateNextBlock) {
			//spdlog::info("[Network] handle UpdateNextBlock: {}", static_cast<char>(updateNextBlock.next));
			wrapperToServer_.Clear();
			auto nextBlock = wrapperToServer_.mutable_next_block();
			nextBlock->set_uuid(player.uuid);
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

		void connect() {
			wrapperToServer_.Clear();
			wrapperToServer_.mutable_connect()->set_name("MW Server");
			send(wrapperToServer_);
		}

		bool isConnected() const {
			return connected_;
		}

	private:
		std::unique_ptr<game::GameRules> gameRules_;
		conc::runtime runtime_;
		std::shared_ptr<conc::manual_executor> manualExecutor_;
		conc::async_lock lock_;
		conc::async_condition_variable cv_;

		bool connected_ = false;
		std::vector<game::PlayerSlot> playerSlots_;
		std::map<int, game::DevicePtr> deviceBySlotIndex_;
		std::map<int, tetris::Ai> aiBySlotIndex_;

		mw::signals::ScopedConnections connections_;

		std::vector<NetworkPlayer> players_;
		
		tp_c2s::Wrapper wrapperToServer_;
		tp_s2c::Wrapper wrapperFromServer_;
		std::shared_ptr<Client> client_;
		std::shared_ptr<game::TetrisGame> tetrisGame_;
		std::string uuid_;
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

	bool Network::isConnected() const {
		return impl_->isConnected();
	}

	void Network::disconnect() {
		impl_->disconnect();
	}

	void Network::connect() {
		impl_->connect();
	}

	bool Network::startGame(std::unique_ptr<game::GameRules> gameRules, int w, int h) {
		impl_->startGame(std::move(gameRules), w, h);
		return true;
	}

	mw::signals::Connection Network::addPlayerSlotListener(std::function<void(game::PlayerSlot, int)> listener) {
		return impl_->playerSlotUpdate.connect(listener);
	}

	mw::signals::Connection Network::addConnectionListener(std::function<void(bool)> listener) {
		return impl_->connected.connect(listener);
	}

}
