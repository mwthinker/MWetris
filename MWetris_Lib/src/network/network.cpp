#include "network.h"
#include "protobufmessagequeue.h"
#include "util.h"
#include "protocol.h"
#include "random.h"
#include "game/remoteplayer.h"
#include "game/localplayerboardbuilder.h"
#include "game/humanplayer.h"
#include "game/aiplayer.h"
#include "game/playerslot.h"
#include "network/client.h"

#include <shared.pb.h>
#include <client_to_server.pb.h>
#include <server_to_client.pb.h>

#include <helper.h>

#include <net/client.h>

#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <concurrencpp/concurrencpp.h>

namespace conc = concurrencpp;

namespace mwetris::network {

	namespace {

		game::LocalPlayerBoardPtr createLocalPlayerBoard(int width, int height, const std::string& name) {
			return game::LocalPlayerBoardBuilder{}
				.withClearedRows(0)
				.withGameOverPosition(0)
				.withPlayerData(game::DefaultPlayerData{
					.level = 1,
					.points = 0
				})
				.withName(name)
				.withMovingBlockType(tetris::randomBlockType())
				.withNextBlockType(tetris::randomBlockType())
				.withWidth(width)
				.withHeight(height)
				.build();
		}

		game::PlayerPtr createPlayer(int width, int height, const game::Human& human) {
			return std::make_shared<game::HumanPlayer>(human.device, createLocalPlayerBoard(width, height, human.name));
		}

		game::PlayerPtr createPlayer(int width, int height, const game::Ai& ai) {
			return std::make_shared<game::AiPlayer>(ai.ai, createLocalPlayerBoard(width, height, ai.name));
		}

		game::RemotePlayerPtr createRemotePlayer(tetris::BlockType current, tetris::BlockType next, const std::string& playerUuid) {
			return std::make_shared<game::RemotePlayer>(current, next, playerUuid);
		}

	}

	Network::Network(std::shared_ptr<Client> client)
		: client_{client} {

		manualExecutor_ = runtime_.make_executor<conc::manual_executor>();
		manualExecutor_->post([this]() {
			stepOnce();
		});
	}

	bool Network::isActive() const {
		return true;
	}

	Network::~Network() {}

	const std::string& Network::getGameRoomUuid() const {
		return gameRoomUuid_;
	}

	void Network::startGame(int w, int h) {
		wrapperToServer_.Clear();
		wrapperToServer_.mutable_start_game()->set_ready(true);
		send(wrapperToServer_);
	}

	void Network::update() {
		manualExecutor_->loop_once();
		cv_.notify_all();
	}

	void Network::sendPause(bool pause) {
		wrapperToServer_.Clear();
		wrapperToServer_.mutable_game_command()->set_pause(pause);
		send(wrapperToServer_);
	}

	void Network::sendRestart() {
		wrapperToServer_.Clear();
		wrapperToServer_.mutable_game_restart();
		send(wrapperToServer_);
	}

	void Network::createGameRoom(const std::string& gameRoom) {
		wrapperToServer_.Clear();
		wrapperToServer_.mutable_create_game_room()->set_name(gameRoom);
		send(wrapperToServer_);
	}

	void Network::leaveRoom() {

	}

	void Network::setPlayerSlot(const game::PlayerSlot& playerSlot, int index) {
		if (index >= networkSlots_.size()) {
			spdlog::error("[Network] Invalid slot index: {}", index);
			return;
		}

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

	void Network::joinGameRoom(const std::string& uuid) {
		connections_.clear();

		wrapperToServer_.Clear();
		auto joinGameRoom = wrapperToServer_.mutable_join_game_room();
		joinGameRoom->set_server_uuid(uuid);
		send(wrapperToServer_);
	}

	bool Network::isInsideRoom() const {
		return !gameRoomUuid_.empty();
	}

	conc::result<void> Network::stepOnce() {
		// Connect to server
		co_await nextMessage();
		if (wrapperFromServer_.has_failed_to_connect()) {
			co_return;
		} else if (wrapperFromServer_.has_game_room_created()) {
			handlGameRoomCreated(wrapperFromServer_.game_room_created());
		} else if (wrapperFromServer_.has_game_room_joined()) {
			handleGameRoomJoined(wrapperFromServer_.game_room_joined());
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

	void Network::handleConnection() {}

	conc::result<void> Network::nextMessage() {
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

	void Network::handleGameRestart(const tp_s2c::GameRestart& gameRestart) {
		auto current = static_cast<tetris::BlockType>(gameRestart.current());
		auto next = static_cast<tetris::BlockType>(gameRestart.next());
		restartEvent(RestartEvent{
			.current = current,
			.next = next
		});
	}

	void Network::handleGameCommand(const tp_s2c::GameCommand& gameCommand) {
		spdlog::info("[Network] Paused: {}", gameCommand.pause() ? "true" : "false");
		pauseEvent(PauseEvent{
			.pause = gameCommand.pause()
		});
	}

	void Network::handlGameRoomCreated(const tp_s2c::GameRoomCreated& gameRoomCreated) {
		gameRoomUuid_ = gameRoomCreated.server_uuid();
		clientUuid_ = gameRoomCreated.client_uuid();
		spdlog::info("[Network] GameRoomCreated: {}", gameRoomUuid_);
		createGameRoomEvent(CreateGameRoomEvent{
			.join = true
		});
	}

	void Network::handleGameRoomJoined(const tp_s2c::GameRoomJoined& gameRoomJoined) {
		spdlog::info("[Network] GameRoomJoined: {}, client uuid: {}", gameRoomJoined.server_uuid(), gameRoomJoined.client_uuid());
		gameRoomUuid_ = "";
		clientUuid_ = gameRoomJoined.client_uuid();
		joinGameRoomEvent(JoinGameRoomEvent{
			.join = true
		});
	}

	void Network::handleGameLooby(const tp_s2c::GameLooby& gameLooby) {
		networkSlots_.clear();
		int index = 0;
		for (const auto& tpSlot : gameLooby.slots()) {
			switch (tpSlot.slot_type()) {
				case tp_s2c::GameLooby_SlotType_REMOTE:
					if (tpSlot.client_uuid() == clientUuid_) {
						if (tpSlot.ai()) {
							networkSlots_.emplace_back(game::Ai{.name = tpSlot.name()}, clientUuid_);
						} else {
							networkSlots_.emplace_back(game::Human{.name = tpSlot.name()}, clientUuid_);
						}
					} else {
						networkSlots_.emplace_back(game::Remote{
							.name = tpSlot.name(),
							.ai = tpSlot.ai()
						}, clientUuid_);
					}
					break;
				case tp_s2c::GameLooby_SlotType_OPEN_SLOT:
					networkSlots_.emplace_back(game::OpenSlot{}, "");
					break;
				case tp_s2c::GameLooby_SlotType_CLOSED_SLOT:
					networkSlots_.emplace_back(game::ClosedSlot{}, "");
					break;
				default:
					continue;
			}
			playerSlotEvent(PlayerSlotEvent{
				.playerSlot = networkSlots_[index].playerSlot,
				.index = index
			});
			++index;
		}
		index = 0;
	}

	void Network::handleConnections(const tp_s2c::Connections& connections) {
		for (const auto& uuid : connections.uuids()) {
			spdlog::info("[Network] Connected uuid: {}", uuid);
		}
	}

	void Network::handleCreateGame(int width, int height, const tp_s2c::CreateGame_Player& tpPlayer, const NetworkSlot& networkSlot) {
		auto current = static_cast<tetris::BlockType>(tpPlayer.current());
		auto next = static_cast<tetris::BlockType>(tpPlayer.next());

		if (auto human = std::get_if<game::Human>(&networkSlot.playerSlot); human) {
			auto& networkPlayer = players_.emplace_back(
				NetworkPlayer{
					.player = createPlayer(width, height, *human),
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
		} else if (auto ai = std::get_if<game::Ai>(&networkSlot.playerSlot); ai) {
			auto& networkPlayer = players_.emplace_back(
				NetworkPlayer{
					.player = createPlayer(height, height, *ai),
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
		} else if (auto remote = std::get_if<game::Remote>(&networkSlot.playerSlot); remote) {
			auto& networkPlayer = players_.emplace_back(
				NetworkPlayer{
					.player = createRemotePlayer(current, next, tpPlayer.player_uuid()),
					.uuid = tpPlayer.player_uuid()
				}
			);
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

	void Network::handleCreateGame(const tp_s2c::CreateGame& createGame) {
		connections_.clear();
		players_.clear();
		fillSlotsWithDevicesAndAis();

		const auto& tpPlayers = createGame.players();

		int index = 0;
		for (const auto& tpPlayer : createGame.players()) {
			if (index >= networkSlots_.size()) {
				// TODO! Handle error!
				break;
			}
			handleCreateGame(createGame.width(), createGame.height(), tpPlayer, networkSlots_[index]);
			++index;
		}

		std::vector<game::PlayerPtr> players;
		for (auto& player : players_) {
			players.push_back(player.player);
		}
		createGameEvent(CreateGameEvent{
			.players = std::move(players)
		});
	}

	void Network::fillSlotsWithDevicesAndAis() {
		for (int i = 0; i < networkSlots_.size(); ++i) {
			if (auto human = std::get_if<game::Human>(&networkSlots_[i].playerSlot); human) {
				human->device = deviceBySlotIndex_[i];
			} else if (auto ai = std::get_if<game::Ai>(&networkSlots_[i].playerSlot); ai) {
				ai->ai = aiBySlotIndex_[i];
			}
		}
	}

	void Network::restartGame(game::GameRestart gameRestart) {
		wrapperToServer_.Clear();
		auto tpGameRestart = wrapperToServer_.mutable_game_restart();
		tpGameRestart->set_current(static_cast<tp::BlockType>(gameRestart.current));
		tpGameRestart->set_next(static_cast<tp::BlockType>(gameRestart.next));
		send(wrapperToServer_);
	}

	void Network::handlePlayerBoardUpdate(const NetworkPlayer& player, const game::UpdateRestart& updateRestart) {
		spdlog::info("[Network] handle UpdateRestart: current={}, next={}", static_cast<char>(updateRestart.current), static_cast<char>(updateRestart.next));
	}

	void Network::handlePlayerBoardUpdate(const NetworkPlayer& player, const game::UpdatePlayerData& updatePlayerData) {
		spdlog::info("[Network] handle UpdatePlayerData");
	}

	void Network::handlePlayerBoardUpdate(const NetworkPlayer& player, const game::ExternalRows& externalRows) {
		spdlog::info("[Network] handle ExternalRows");
	}

	void Network::handlePlayerBoardUpdate(const NetworkPlayer& player, const game::UpdateMove& updateMove) {
		//spdlog::info("[Network] handle UpdateMove: {}", static_cast<int>(updateMove.move));
		wrapperToServer_.Clear();
		auto boardMove = wrapperToServer_.mutable_board_move();
		//auto uuid = player.getUuid();
		//boardMove->set_player_uuid(player.getUuid());
		boardMove->set_move(static_cast<tp::Move>(updateMove.move));
		boardMove->set_player_uuid(player.uuid);
		send(wrapperToServer_);
	}

	void Network::handlePlayerBoardUpdate(const NetworkPlayer& player, const game::UpdateNextBlock& updateNextBlock) {
		//spdlog::info("[Network] handle UpdateNextBlock: {}", static_cast<char>(updateNextBlock.next));
		wrapperToServer_.Clear();
		auto nextBlock = wrapperToServer_.mutable_next_block();
		nextBlock->set_uuid(player.uuid);
		nextBlock->set_next(static_cast<tp::BlockType>(updateNextBlock.next));
		send(wrapperToServer_);
	}

	void Network::handleBoardEvent(const game::Player& player, tetris::BoardEvent boardEvent, int nbr) {
		spdlog::info("[Network] handle UpdateMove: {}, {}", static_cast<int>(boardEvent), nbr);
	}

	void Network::send(const tp_c2s::Wrapper& wrapper) {
		ProtobufMessage message;
		client_->acquire(message);
		message.setBuffer(wrapper);
		client_->send(std::move(message));
	}

}
