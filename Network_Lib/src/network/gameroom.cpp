#include "gameroom.h"
#include "id.h"

#include <protocol/server_to_client.pb.h>
#include <protocol/client_to_server.pb.h>

#include <tetris/helper.h>

#include <spdlog/spdlog.h>

namespace network {

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
						fromCppToProto(slot.playerId, *tpSlot->mutable_player_id());
						fromCppToProto(slot.clientId, *tpSlot->mutable_client_id());
						break;
					case SlotType::Closed:
						tpSlot->set_slot_type(tp_s2c::GameLooby_SlotType_CLOSED_SLOT);
						break;
					default:
						spdlog::error("[DebugServer.cpp] Invalid slot type");
				}
			}
		}

		bool hasPlayers(const std::vector<Slot>& playerSlots, const ClientId& clientId) {
			for (const auto& slot : playerSlots) {
				if (slot.type == SlotType::Remote && slot.clientId == clientId) {
					return true;
				}
			}
			return false;
		}

		std::list<int> createIds(int nbr) {
			std::list<int> ids;
			for (int i = 0; i < nbr; ++i) {
				ids.push_back(i);
			}
			return ids;
		}

		std::vector<GameRoomClient>::iterator findClient(std::vector<GameRoomClient>& connectedClients, const ClientId& clientId) {
			if (auto it = std::find_if(connectedClients.begin(), connectedClients.end(), [&clientId](const auto& client) {
				return client.clientId == clientId;
			}); it != connectedClients.end()) {
				return it;
			}
			return connectedClients.end();
		}

		void fromCppToProto(const std::vector<GameRoomClient>& connectedClients, tp_s2c::GameRoomClients& tpGameRoomClients) {
			for (const auto& client : connectedClients) {
				auto tpClient = tpGameRoomClients.add_clients();
				tpClient->set_connection_id(client.connectionId);
				fromCppToProto(client.clientId, *tpClient->mutable_client_id());
			}
		}

	}

	GameRoom::GameRoom()
		: GameRoom{"", false} {
	}

	GameRoom::GameRoom(const std::string& name, bool isPublic)
		: name_{name}
		, isPublic_{isPublic}
		, connectionIds_{createIds(7)} {
		
		gameRoomId_ = GameRoomId::generateUniqueId();
		playerSlots_.resize(4, Slot{.type = SlotType::Open});
		gameRules_.mutable_default_game_rules()->set_start_level(0);
		gameRules_.mutable_default_game_rules()->set_lines_per_level(10);
		gameRules_.mutable_default_game_rules()->set_max_level(40);
	}

	GameRoom::~GameRoom() {}

	void GameRoom::sendToAllClients(Server& server, const tp_s2c::Wrapper& message, const ClientId& exceptClientId) {
		for (const auto& [clientId, _] : connectedClients_) {
			if (clientId == exceptClientId) {
				continue;
			}
			server.sendToClient(clientId, message);
		}
	}

	const std::string& GameRoom::getName() const {
		return name_;
	}

	bool GameRoom::isPublic() const {
		return isPublic_;
	}

	const GameRoomId& GameRoom::getGameRoomId() const {
		return gameRoomId_;
	}

	bool GameRoom::isFull() const {
		return connectedClients_.size() >= 4;
	}

	const std::vector<GameRoomClient>& GameRoom::getConnectedClientIds() const {
		return connectedClients_;
	}

	int GameRoom::getConnectedClientSize() const {
		return static_cast<int>(connectedClients_.size());
	}

	void GameRoom::disconnect(Server& server, const ClientId& clientId) {
		auto connectedClients = connectedClients_;
		std::erase_if(connectedClients, [&](const auto& client) {
			if (client.clientId == clientId) {
				// Return connection id to be reused for new client connections
				connectionIds_.push_back(client.connectionId);
				return true;
			}

			return false;
		});
		
		wrapperToClient_.Clear();
		auto leaveGameRoom = wrapperToClient_.mutable_leave_game_room();
		fromCppToProto(gameRoomId_, *leaveGameRoom->mutable_game_room_id());
		fromCppToProto(clientId, *leaveGameRoom->mutable_client_id());
		fromCppToProto(connectedClients, *leaveGameRoom->mutable_game_room_clients());
		sendToAllClients(server, wrapperToClient_);

		connectedClients_ = connectedClients;
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

	void GameRoom::receiveMessage(Server& server, const ClientId& clientId, const tp_c2s::Wrapper& wrapperFromClient) {
		wrapperToClient_.Clear();

		if (wrapperFromClient.has_create_game_room()) {
			handleCreateGameRoom(server, clientId, wrapperFromClient.create_game_room());
		}
		if (wrapperFromClient.has_join_game_room()) {
			handleJoinGameRoom(server, clientId, wrapperFromClient.join_game_room());
		}
		if (wrapperFromClient.has_player_slot()) {
			handlePlayerSlot(server, clientId, wrapperFromClient.player_slot());
		}
		if (wrapperFromClient.has_game_command()) {
			handleGameCommand(server, wrapperFromClient.game_command());
		}
		if (wrapperFromClient.has_start_game()) {
			handleStartGame(server, clientId, wrapperFromClient.start_game());
		}
		if (wrapperFromClient.has_board_move()) {
			handleBoardMove(server, clientId, wrapperFromClient.board_move());
		}
		if (wrapperFromClient.has_next_block()) {
			handleBoardNextBlock(server, clientId, wrapperFromClient.next_block());
		}
		if (wrapperFromClient.has_board_external_squares()) {
			handleBoardExternalSquares(server, clientId, wrapperFromClient.board_external_squares());
		}
		if (wrapperFromClient.has_game_restart()) {
			handleGameRestart(server, clientId, wrapperFromClient.game_restart());
		}
		if (wrapperFromClient.has_request_game_restart()) {
			handleRequestGameRestart(server, clientId, wrapperFromClient.request_game_restart());
		}
		if (wrapperFromClient.has_remove_client()) {
			handleRemoveClient(server, clientId, wrapperFromClient.remove_client());
		}

		server.triggerPlayerSlotEvent(playerSlots_);
	}

	void GameRoom::handlePlayerSlot(Server& server, const ClientId& clientId, const tp_c2s::PlayerSlot& tpPlayerSlot) {
		int index = tpPlayerSlot.index();

		if (index < 0 || index >= playerSlots_.size()) {
			spdlog::error("Invalid index {}", index);
			return;
		}

		if (slotBelongsToClient(clientId, index)) {
			if (tpPlayerSlot.slot_type() == tp_c2s::PlayerSlot_SlotType_OPEN_SLOT) {
				playerSlots_[index] = Slot{.type = SlotType::Open};
			} else {
				playerSlots_[index] = Slot{
					.clientId = clientId,
					.playerId = PlayerId::generateUniqueId(),
					.name = tpPlayerSlot.name(),
					.ai = tpPlayerSlot.slot_type() == tp_c2s::PlayerSlot_SlotType_AI,
					.type = SlotType::Remote
				};
			}
		}

		wrapperToClient_.Clear();
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

	void GameRoom::handleStartGame(Server& server, const ClientId& clientId, const tp_c2s::StartGame& startGame) {
		auto createGame = wrapperToClient_.mutable_create_game();
		createGame->set_width(10);
		createGame->set_height(24);
		gameRules_.CopyFrom(startGame.game_rules());
		createGame->mutable_game_rules()->CopyFrom(gameRules_);

		auto current = tetris::randomBlockType();
		auto next = tetris::randomBlockType();

		for (const auto& slot : playerSlots_) {
			if (slot.type == SlotType::Remote) {
				auto tpRemotePlayer = createGame->add_players();
				fromCppToProto(slot.clientId, *tpRemotePlayer->mutable_client_id());
				fromCppToProto(slot.playerId, *tpRemotePlayer->mutable_player_id());
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

	void GameRoom::handleBoardMove(Server& server, const ClientId& clientId, const tp_c2s::BoardMove& boardMove) {
		auto move = static_cast<tetris::Move>(boardMove.move());
		// TODO! Confirm player id belongs to correct client
		PlayerId playerId = boardMove.player_id();

		wrapperToClient_.Clear();
		auto boardMoveToClient = wrapperToClient_.mutable_board_move();
		boardMoveToClient->set_move(boardMove.move());
		fromCppToProto(playerId, *boardMoveToClient->mutable_player_id());
		sendToAllClients(server, wrapperToClient_, clientId);
	}

	void GameRoom::handleBoardNextBlock(Server& server, const ClientId& clientId, const tp_c2s::BoardNextBlock& boardNextBlock) {
		// TODO! Confirm player id belongs to correct client

		PlayerId playerId = boardNextBlock.player_id();

		wrapperToClient_.Clear();
		auto boardNextBlockToClient = wrapperToClient_.mutable_next_block();
		boardNextBlockToClient->set_next(boardNextBlock.next());
		fromCppToProto(playerId, *boardNextBlockToClient->mutable_player_id());

		sendToAllClients(server, wrapperToClient_, clientId);
	}

	void GameRoom::handleBoardExternalSquares(Server& server, const ClientId& clientId, const tp_c2s::BoardExternalSquares& boardExternalSquares) {
		wrapperToClient_.Clear();
		auto boardExternalSquaresToClient = wrapperToClient_.mutable_board_external_squares();
		for (const auto& blockType : boardExternalSquares.block_types()) {
			boardExternalSquaresToClient->add_block_types(static_cast<tp::BlockType>(blockType));
		}
		PlayerId playerId = boardExternalSquares.player_id();
		fromCppToProto(playerId, *boardExternalSquaresToClient->mutable_player_id());
		sendToAllClients(server, wrapperToClient_, clientId);
	}

	void GameRoom::handleRequestGameRestart(Server& server, const ClientId& clientId, const tp_c2s::RequestGameRestart& requestGameRestart) {
		auto current = tetris::randomBlockType();
		auto next = tetris::randomBlockType();
		auto requestGameRestartToClient = wrapperToClient_.mutable_request_game_restart();
		requestGameRestartToClient->set_current(static_cast<tp::BlockType>(current));
		requestGameRestartToClient->set_next(static_cast<tp::BlockType>(next));
		sendToAllClients(server, wrapperToClient_);
		wrapperToClient_.Clear();
	}

	void GameRoom::handleGameRestart(Server& server, const ClientId& clientId, const tp_c2s::GameRestart& gameRestart) {
		auto gameRestartToClient = wrapperToClient_.mutable_game_restart();
		gameRestartToClient->set_current(static_cast<tp::BlockType>(gameRestart.current()));
		gameRestartToClient->set_next(static_cast<tp::BlockType>(gameRestart.next()));
		fromCppToProto(clientId, *gameRestartToClient->mutable_client_id());

		sendToAllClients(server, wrapperToClient_, clientId);
		wrapperToClient_.Clear();
	}

	void GameRoom::handleCreateGameRoom(Server& server, const ClientId& clientId, const tp_c2s::CreateGameRoom& createGameRoom) {
		name_ = createGameRoom.name();

		sendJoinGameRoom(server, clientId);
	}

	void GameRoom::handleJoinGameRoom(Server& server, const ClientId& clientId, const tp_c2s::JoinGameRoom& joinGameRoom) {
		sendJoinGameRoom(server, clientId);
	}

	void GameRoom::sendJoinGameRoom(Server& server, const ClientId& clientId) {
		const auto& newClient = connectedClients_.emplace_back(GameRoomClient{
			.clientId = clientId,
			.connectionId = connectionIds_.front()
		});
		connectionIds_.pop_front();

		auto gameRoomJoined = wrapperToClient_.mutable_game_room_joined();
		fromCppToProto(gameRoomId_, *gameRoomJoined->mutable_game_room_id());
		fromCppToProto(newClient.clientId, *gameRoomJoined->mutable_client_id());
		fromCppToProto(connectedClients_ , *gameRoomJoined->mutable_game_room_clients());
		
		addPlayerSlotsToGameLooby(*gameRoomJoined->mutable_game_looby(), playerSlots_);

		sendToAllClients(server, wrapperToClient_);
	}

	void GameRoom::handleLeaveGameRoom(Server& server, const ClientId& clientId, const tp_c2s::LeaveGameRoom& leaveGameRoom) {
		if (auto it = findClient(connectedClients_, clientId); it != connectedClients_.end()) {
			connectedClients_.erase(it);

			wrapperToClient_.Clear();
			auto leaveGameRoom = wrapperToClient_.mutable_leave_game_room();
			fromCppToProto(gameRoomId_, *leaveGameRoom->mutable_game_room_id());
			fromCppToProto(clientId, *leaveGameRoom->mutable_client_id());
			sendToAllClients(server, wrapperToClient_);
		} else {
			spdlog::error("Client {} not found in connected clients", clientId);
		}
	}

	void GameRoom::removeClientFromGameRoom(Server& server, const ClientId& clientId) {
		if (auto it = findClient(connectedClients_, clientId); it != connectedClients_.end()) {
			connectedClients_.erase(it);
			
			wrapperToClient_.Clear();
			auto removeClient = wrapperToClient_.mutable_remove_client();
			fromCppToProto(clientId, *removeClient->mutable_client_id());
			sendToAllClients(server, wrapperToClient_);
		} else {
			spdlog::error("Client {} not found in connected clients", clientId);
		}
	}

	void GameRoom::handleRemoveClient(Server& server, const ClientId& clientId, const tp_c2s::RemoveClient& removeClient) {
		ClientId removeClientId = removeClient.client_id();
		removeClientFromGameRoom(server, removeClientId);
	}

	bool GameRoom::slotBelongsToClient(const ClientId& clientId, int slotIndex) const {
		const auto& slot = playerSlots_.at(slotIndex);
		if (slot.type == SlotType::Remote && slot.clientId == clientId) {
			return true;
		}
		return slot.type == SlotType::Open;
	}

}
