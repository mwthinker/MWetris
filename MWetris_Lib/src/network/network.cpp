#include "network.h"
#include "protobufmessagequeue.h"
#include "util.h"
#include "protocol.h"
#include "random.h"
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

	Network::Network(std::shared_ptr<Client> client)
		: client_{client} {

		manualExecutor_ = runtime_.make_executor<conc::manual_executor>();
		manualExecutor_->post([this]() {
			stepOnce();
		});
	}

	bool Network::isInsideGameRoom() const {
		return !gameRoomUuid_.empty();
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
		auto requestGameRestart = wrapperToServer_.mutable_request_game_restart();
		requestGameRestart->set_restart(true);
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
			if (wrapperFromServer_.has_request_game_restart()) {
				handleRequestGameRestart(wrapperFromServer_.request_game_restart());
			}
			if (wrapperFromServer_.has_game_restart()) {
				handleGameRestart(wrapperFromServer_.game_restart());
			}
			if (wrapperFromServer_.has_board_move()) {
				handleBoardMove(wrapperFromServer_.board_move());
			}
			if (wrapperFromServer_.has_next_block()) {
				handleBoardNextBlock(wrapperFromServer_.next_block());
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

	void Network::handleRequestGameRestart(const tp_s2c::RequestGameRestart& requestGameRestart) {
		spdlog::info("[Network] RequestGameRestart");
		if (std::none_of(players_.begin(), players_.end(), [this](const NetworkPlayer& networkPlayer) {
			return networkPlayer.player->isLocal();
		})) {
			spdlog::debug("[Network] Ignore RequestGameRestart, no local players for client {}", clientUuid_);
			return;
		}
		
		wrapperToServer_.Clear();
		auto gameRestartToServer = wrapperToServer_.mutable_game_restart();
		gameRestartToServer->set_current(requestGameRestart.current());
		gameRestartToServer->set_next(requestGameRestart.next());
		send(wrapperToServer_);

		for (auto& networkPlayer : players_) {
			if (networkPlayer.player->isLocal()) {
				networkPlayer.player->updateRestart(static_cast<tetris::BlockType>(requestGameRestart.current()), static_cast<tetris::BlockType>(requestGameRestart.next()));
			}
		}
	}

	void Network::handleGameRestart(const tp_s2c::GameRestart& gameRestart) {
		auto current = static_cast<tetris::BlockType>(gameRestart.current());
		auto next = static_cast<tetris::BlockType>(gameRestart.next());
		
		const auto& clientUuid = gameRestart.client_uuid();
		if (clientUuid == clientUuid_) {
			spdlog::warn("[Network] Client {} received own GameRestart message. Ignoring!", clientUuid_);
			return;
		}

		for (auto& networkPlayer : players_) {
			if (networkPlayer.clientId == clientUuid) {
				networkPlayer.player->updateRestart(current, next);
			}
		}
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
		spdlog::info("[Network] GameRoomCreated: {}, client uuid: {}", gameRoomUuid_, clientUuid_);
		createGameRoomEvent(CreateGameRoomEvent{
			.join = true
		});
		handleGameLooby(gameRoomCreated.game_looby());
	}

	void Network::handleGameRoomJoined(const tp_s2c::GameRoomJoined& gameRoomJoined) {
		spdlog::info("[Network] GameRoomJoined: {}, client uuid: {}", gameRoomJoined.server_uuid(), gameRoomJoined.client_uuid());
		gameRoomUuid_ = gameRoomJoined.server_uuid();
		clientUuid_ = gameRoomJoined.client_uuid();
		joinGameRoomEvent(JoinGameRoomEvent{
			.join = true
		});
		handleGameLooby(gameRoomJoined.game_looby());
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

		tetris::TetrisBoard tetrisBoard{width, height, current, next};
		game::DefaultPlayerData playerData{
			.level = tpPlayer.level(),
			.points = tpPlayer.points()
		};
		NetworkPlayer networkPlayer;
		if (auto human = std::get_if<game::Human>(&networkSlot.playerSlot); human) {
			networkPlayer = players_.emplace_back(
				NetworkPlayer{
					.player = game::createHumanPlayer(human->device, playerData, std::move(tetrisBoard)),
					.uuid = tpPlayer.player_uuid(),
					.clientId = clientUuid_
				}
			);
		} else if (auto ai = std::get_if<game::Ai>(&networkSlot.playerSlot); ai) {
			networkPlayer = players_.emplace_back(
				NetworkPlayer{
					.player = game::createAiPlayer(ai->ai, playerData, std::move(tetrisBoard)),
					.uuid = tpPlayer.player_uuid(),
					.clientId = clientUuid_
				}
			);
		} else if (auto remote = std::get_if<game::Remote>(&networkSlot.playerSlot); remote) {
			networkPlayer = players_.emplace_back(
				NetworkPlayer{
					.player = game::createRemotePlayer(playerData, std::move(tetrisBoard)),
					.uuid = tpPlayer.player_uuid(),
					.clientId = tpPlayer.client_uuid()
				}
			);
		}
		if (networkPlayer.player) {
			connections_ += networkPlayer.player->playerBoardUpdate.connect([this, networkPlayer](game::PlayerBoardEvent playerBoardEvent) {
				std::visit([&](auto&& event) {
					handlePlayerBoardUpdate(networkPlayer, event);
				}, playerBoardEvent);
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

	void Network::handleBoardMove(const tp_s2c::BoardMove& boardMove) {
		auto move = static_cast<tetris::Move>(boardMove.move());
		for (auto& networkPlayer : players_) {
			if (networkPlayer.uuid == boardMove.uuid()) {
				if (networkPlayer.player->isRemote()) {
					networkPlayer.player->updateMove(move);
				} else {
					spdlog::error("[Network] Invalid player type for BoardMove");
				}
			}
		}
	}

	void Network::handleBoardNextBlock(const tp_s2c::BoardNextBlock& boardNextBlock) {
		for (auto& networkPlayer : players_) {
			if (networkPlayer.uuid == boardNextBlock.uuid()) {
				if (networkPlayer.player->isRemote()) {
					auto next = static_cast<tetris::BlockType>(boardNextBlock.next());
					networkPlayer.player->updateNextBlock(next);
				} else {
					spdlog::error("[Network] Invalid player type for BoardNextBlock");
				}
			}
		}
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

	void Network::handlePlayerBoardUpdate(const NetworkPlayer& player, const game::TetrisBoardEvent& tetrisBoardEvent) {
		spdlog::info("[Network] handle TetrisBoardEvent");
	}

	void Network::send(const tp_c2s::Wrapper& wrapper) {
		ProtobufMessage message;
		client_->acquire(message);
		message.setBuffer(wrapper);
		client_->send(std::move(message));
	}

}
