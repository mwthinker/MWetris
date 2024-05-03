#include "gameroom.h"
#include "util/uuid.h"

#include "debugserver.h"
#include "game/remoteplayer.h"
#include "util/uuid.h"

#include <helper.h>

#include <server_to_client.pb.h>
#include <client_to_server.pb.h>

#include <spdlog/spdlog.h>

namespace mwetris::network {

	namespace {

		void addPlayerSlotsToGameLooby(tp_s2c::GameLooby& gameLooby, const std::vector<Slot>& playerSlots) {
			for (const auto& slot : playerSlots) {
				auto tpSlot = gameLooby.add_slots();
				tpSlot->set_slot_type(tp_s2c::GameLooby_SlotType_UNSPECIFIED_SLOT_TYPE);

				switch (slot.type) {
					case SlotType::Open:
						tpSlot->set_slot_type(tp_s2c::GameLooby_SlotType_OPEN_SLOT);
						break;
					case SlotType::Remote:
						tpSlot->set_slot_type(tp_s2c::GameLooby_SlotType_REMOTE);
						tpSlot->set_ai(slot.ai);
						tpSlot->set_name(slot.name);
						tpSlot->set_player_uuid(slot.playerUuid);
						tpSlot->set_client_uuid(slot.clientUuid);
						break;
					case SlotType::Closed:
						tpSlot->set_slot_type(tp_s2c::GameLooby_SlotType_CLOSED_SLOT);
						break;
					default:
						spdlog::error("[DebugServer.cpp] Invalid slot type");
				}
			}
		}

		bool hasPlayers(const std::vector<Slot>& playerSlots, const std::string& clientUuid) {
			for (const auto& slot : playerSlots) {
				if (slot.type == SlotType::Remote && slot.clientUuid == clientUuid) {
					return true;
				}
			}
			return false;
		}

	}


	GameRoom::GameRoom() {
		uuid_ = util::generateUuid();
		playerSlots_.resize(4, Slot{.type = SlotType::Open});
	}

	GameRoom::~GameRoom() {}

	void GameRoom::sendToAllClients(Server& server, const tp_s2c::Wrapper& message, const std::string& exceptClientUuid) {
		for (const auto& uuid : connectedClientUuids_) {
			if (uuid == exceptClientUuid) {
				continue;
			}
			server.sendToClient(uuid, message);
		}
	}

	const std::string& GameRoom::getName() const {
		return name_;
	}

	const std::string& GameRoom::getUuid() const {
		return uuid_;
	}

	const std::vector<std::string>& GameRoom::getConnectedClientUuids() const {
		return connectedClientUuids_;
	}

	void GameRoom::disconnect(Server& server, const std::string& uuid) {
		
	}

	void GameRoom::sendPause(Server& server, bool pause) {
		paused_ = pause;
		wrapperToClient_.Clear();
		wrapperToClient_.mutable_game_command()->set_pause(pause);
		sendToAllClients(server, wrapperToClient_);
	}

	bool GameRoom::isPaused() const {
		return paused_;
	}

	void GameRoom::requestRestartGame(Server& server) {
		// TODO!
		wrapperToClient_.Clear();
		auto gameRestart = wrapperToClient_.mutable_game_restart();
		gameRestart->set_current(static_cast<tp::BlockType>(tetris::randomBlockType()));
		gameRestart->set_next(static_cast<tp::BlockType>(tetris::randomBlockType()));
		sendToAllClients(server, wrapperToClient_);
	}

	void GameRoom::receiveMessage(Server& server, const std::string& clientUuid, const tp_c2s::Wrapper& wrapperFromClient) {
		wrapperToClient_.Clear();

		if (wrapperFromClient.has_create_game_room()) {
			handleCreateGameRoom(server, clientUuid, wrapperFromClient.create_game_room());
		}
		if (wrapperFromClient.has_join_game_room()) {
			handleJoinGameRoom(server, clientUuid, wrapperFromClient.join_game_room());
		}
		if (wrapperFromClient.has_player_slot()) {
			handlePlayerSlot(server, clientUuid, wrapperFromClient.player_slot());
		}
		if (wrapperFromClient.has_game_command()) {
			handleGameCommand(server, wrapperFromClient.game_command());
		}
		if (wrapperFromClient.has_start_game()) {
			handleStartGame(server, clientUuid, wrapperFromClient.start_game());
		}
		if (wrapperFromClient.has_board_move()) {
			handleBoardMove(server, clientUuid, wrapperFromClient.board_move());
		}
		if (wrapperFromClient.has_next_block()) {
			handleBoardNextBlock(server, clientUuid, wrapperFromClient.next_block());
		}
		if (wrapperFromClient.has_board_external_squares()) {
			handleBoardExternalSquares(server, clientUuid, wrapperFromClient.board_external_squares());
		}
		if (wrapperFromClient.has_game_restart()) {
			handleGameRestart(server, clientUuid, wrapperFromClient.game_restart());
		}
		if (wrapperFromClient.has_request_game_restart()) {
			handleRequestGameRestart(server, clientUuid, wrapperFromClient.request_game_restart());
		}

		server.triggerPlayerSlotEvent(playerSlots_);
	}

	void GameRoom::handlePlayerSlot(Server& server, const std::string& clientUuid, const tp_c2s::PlayerSlot& tpPlayerSlot) {
		int index = tpPlayerSlot.index();

		if (index < 0 || index >= playerSlots_.size()) {
			spdlog::error("Invalid index {}", index);
			return;
		}

		if (slotBelongsToClient(clientUuid, index)) {
			if (tpPlayerSlot.slot_type() == tp_c2s::PlayerSlot_SlotType_OPEN_SLOT) {
				playerSlots_[index] = Slot{.type = SlotType::Open};
			} else {
				playerSlots_[index] = Slot{
					.clientUuid = clientUuid,
					.playerUuid = util::generateUuid(),
					.name = tpPlayerSlot.name(),
					.ai = tpPlayerSlot.slot_type() == tp_c2s::PlayerSlot_SlotType_AI,
					.type = SlotType::Remote
				};
			}
		}

		auto tpGameLooby = wrapperToClient_.mutable_game_looby();
		addPlayerSlotsToGameLooby(*tpGameLooby, playerSlots_);
		sendToAllClients(server, wrapperToClient_);
	}

	void GameRoom::handleGameCommand(Server& server, const tp_c2s::GameCommand& gameCommand) {
		paused_ = gameCommand.pause();
		wrapperToClient_.Clear();
		wrapperToClient_.mutable_game_command()->set_pause(paused_);
		sendToAllClients(server, wrapperToClient_);
	}

	void GameRoom::handleStartGame(Server& server, const std::string& clientUuid, const tp_c2s::StartGame& createServerGame) {
		auto createGame = wrapperToClient_.mutable_create_game();
		createGame->set_width(10);
		createGame->set_height(24);

		auto current = tetris::randomBlockType();
		auto next = tetris::randomBlockType();

		std::vector<game::PlayerBoardPtr> playerBoards;
		for (const auto& slot : playerSlots_) {
			if (slot.type == SlotType::Remote) {
				auto tpRemotePlayer = createGame->add_players();
				tpRemotePlayer->set_client_uuid(slot.clientUuid);
				tpRemotePlayer->set_player_uuid(slot.playerUuid);
				tpRemotePlayer->set_name(slot.name);
				tpRemotePlayer->set_level(0);
				tpRemotePlayer->set_points(0);
				tpRemotePlayer->set_ai(slot.ai);
				tpRemotePlayer->set_current(static_cast<tp::BlockType>(current));
				tpRemotePlayer->set_next(static_cast<tp::BlockType>(next));
			}
		}
		sendToAllClients(server, wrapperToClient_);
	}

	void GameRoom::handleBoardMove(Server& server, const std::string& clientUuid, const tp_c2s::BoardMove& boardMove) {
		auto move = static_cast<tetris::Move>(boardMove.move());
		// TODO! Confirm player uuid belongs to correct client
		const auto& uuid = boardMove.player_uuid();

		wrapperToClient_.Clear();
		auto boardMoveToClient = wrapperToClient_.mutable_board_move();
		boardMoveToClient->set_move(boardMove.move());
		boardMoveToClient->set_uuid(uuid);
		sendToAllClients(server, wrapperToClient_, clientUuid);
	}

	void GameRoom::handleBoardNextBlock(Server& server, const std::string& clientUuid, const tp_c2s::BoardNextBlock& boardNextBlock) {
		// TODO! Confirm player uuid belongs to correct client

		wrapperToClient_.Clear();
		auto boardNextBlockToClient = wrapperToClient_.mutable_next_block();
		boardNextBlockToClient->set_next(boardNextBlock.next());
		boardNextBlockToClient->set_uuid(boardNextBlock.uuid());

		sendToAllClients(server, wrapperToClient_, clientUuid);
	}

	void GameRoom::handleBoardExternalSquares(Server& server, const std::string& clientUuid, const tp_c2s::BoardExternalSquares& boardExternalSquares) {
		static std::vector<tetris::BlockType> blockTypes;
		blockTypes.clear();
		for (const auto& tpBlockType : boardExternalSquares.block_types()) {
			blockTypes.push_back(static_cast<tetris::BlockType>(tpBlockType));
		}
	}

	void GameRoom::handleRequestGameRestart(Server& server, const std::string& clientUuid, const tp_c2s::RequestGameRestart& requestGameRestart) {
		auto current = tetris::randomBlockType();
		auto next = tetris::randomBlockType();
		auto requestGameRestartToClient = wrapperToClient_.mutable_request_game_restart();
		requestGameRestartToClient->set_current(static_cast<tp::BlockType>(current));
		requestGameRestartToClient->set_next(static_cast<tp::BlockType>(next));
		sendToAllClients(server, wrapperToClient_);
		wrapperToClient_.Clear();
	}

	void GameRoom::handleGameRestart(Server& server, const std::string& clientUuid, const tp_c2s::GameRestart& gameRestart) {
		auto gameRestartToClient = wrapperToClient_.mutable_game_restart();
		gameRestartToClient->set_current(static_cast<tp::BlockType>(gameRestart.current()));
		gameRestartToClient->set_next(static_cast<tp::BlockType>(gameRestart.next()));
		gameRestartToClient->set_client_uuid(clientUuid);

		sendToAllClients(server, wrapperToClient_, clientUuid);
		wrapperToClient_.Clear();
	}

	void GameRoom::handleCreateGameRoom(Server& server, const std::string& clientUuid, const tp_c2s::CreateGameRoom& createGameRoom) {
		connectedClientUuids_.push_back(clientUuid);
		name_ = createGameRoom.name();

		auto gameRomeCreated = wrapperToClient_.mutable_game_room_created();
		gameRomeCreated->set_server_uuid(uuid_);
		gameRomeCreated->set_client_uuid(clientUuid);
		addPlayerSlotsToGameLooby(*gameRomeCreated->mutable_game_looby(), playerSlots_);
		
		server.sendToClient(clientUuid, wrapperToClient_);
	}

	void GameRoom::handleJoinGameRoom(Server& server, const std::string& clientUuid, const tp_c2s::JoinGameRoom& joinGameRoom) {
		connectedClientUuids_.push_back(clientUuid);
		
		auto gameRoomJoined = wrapperToClient_.mutable_game_room_joined();
		gameRoomJoined->set_server_uuid(joinGameRoom.server_uuid());
		gameRoomJoined->set_client_uuid(clientUuid);
		addPlayerSlotsToGameLooby(*gameRoomJoined->mutable_game_looby(), playerSlots_);

		server.sendToClient(clientUuid, wrapperToClient_);
	}

	bool GameRoom::slotBelongsToClient(const std::string& clientUuid, int slotIndex) const {
		const auto& slot = playerSlots_.at(slotIndex);
		if (slot.type == SlotType::Remote && slot.clientUuid == clientUuid) {
			return true;
		}
		return slot.type == SlotType::Open;
	}

}
