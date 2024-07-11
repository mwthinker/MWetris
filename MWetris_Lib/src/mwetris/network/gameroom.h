#ifndef MWETRIS_NETWORK_GAMEROOM_H
#define MWETRIS_NETWORK_GAMEROOM_H

#include "server.h"
#include "id.h"
#include "networkevent.h"

#include <server_to_client.pb.h>
#include <client_to_server.pb.h>

#include <map>
#include <string>
#include <vector>

namespace mwetris::network {

	class GameRoom {
	public:
		GameRoom();

		GameRoom(const std::string& name, bool isPublic);

		~GameRoom();

		void sendToAllClients(Server& server, const tp_s2c::Wrapper& message, const ClientId& exceptClientId = ClientId{std::string{}});

		const std::string& getName() const;

		bool isPublic() const;

		const GameRoomId& getGameRoomId() const;

		bool isFull() const;

		const std::vector<GameRoomClient>& getConnectedClientIds() const;
		int getConnectedClientSize() const;

		void disconnect(Server& server, const ClientId& clientId);

		void sendPause(Server& server, bool pause);

		bool isPaused() const;

		void requestRestartGame(Server& server);

		void receiveMessage(Server& server, const ClientId& clientId, const tp_c2s::Wrapper& wrapperFromClient);

		void removeClientFromGameRoom(Server& server, const ClientId& clientId);

	private:
		void handlePlayerSlot(Server& server, const ClientId& clientId, const tp_c2s::PlayerSlot& tpPlayerSlot);

		void handleGameCommand(Server& server, const tp_c2s::GameCommand& gameCommand);

		void handleStartGame(Server& server, const ClientId& clientId, const tp_c2s::StartGame& createServerGame);

		void handleBoardMove(Server& server, const ClientId& clientId, const tp_c2s::BoardMove& boardMove);

		void handleBoardNextBlock(Server& server, const ClientId& clientId, const tp_c2s::BoardNextBlock& boardNextBlock);

		void handleBoardExternalSquares(Server& server, const ClientId& clientId, const tp_c2s::BoardExternalSquares& boardExternalSquares);

		void handleRequestGameRestart(Server& server, const ClientId& clientId, const tp_c2s::RequestGameRestart& requestGameRestart);

		void handleGameRestart(Server& server, const ClientId& clientId, const tp_c2s::GameRestart& gameRestart);

		void handleCreateGameRoom(Server& server, const ClientId& clientId, const tp_c2s::CreateGameRoom& createGameRoom);

		void handleJoinGameRoom(Server& server, const ClientId& clientId, const tp_c2s::JoinGameRoom& joinGameRoom);

		void handleLeaveGameRoom(Server& server, const ClientId& clientId, const tp_c2s::LeaveGameRoom& leaveGameRoom);

		void handleRemoveClient(Server& server, const ClientId& clientId, const tp_c2s::RemoveClient& removeClient);

		bool slotBelongsToClient(const ClientId& clientId, int slotIndex) const;

		void sendJoinGameRoom(Server& server, const ClientId& clientId);

		std::string name_;
		GameRoomId gameRoomId_;

		std::vector<Slot> playerSlots_;
		std::vector<GameRoomClient> connectedClients_;
		bool paused_ = false;
		bool isPublic_ = false;

		tp_s2c::Wrapper wrapperToClient_;
		tp::GameRules gameRules_;
		std::list<int> connectionIds_;
	};

}

#endif
