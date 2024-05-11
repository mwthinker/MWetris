#ifndef MWETRIS_NETWORK_GAMEROOM_H
#define MWETRIS_NETWORK_GAMEROOM_H

#include "debugserver.h"
#include "server.h"
#include "id.h"

#include <server_to_client.pb.h>
#include <client_to_server.pb.h>

#include <map>
#include <string>
#include <vector>

namespace mwetris::network {

	class GameRoom {
	public:
		GameRoom();

		~GameRoom();

		void sendToAllClients(Server& server, const tp_s2c::Wrapper& message, const ClientId& exceptClientUuid = ClientId{std::string{}});

		const std::string& getName() const;

		const GameRoomId& getGameRoomId() const;

		const std::vector<ClientId>& getConnectedClientUuids() const;

		void disconnect(Server& server);

		void sendPause(Server& server, bool pause);

		bool isPaused() const;

		void requestRestartGame(Server& server);

		void receiveMessage(Server& server, const ClientId& clientUuid, const tp_c2s::Wrapper& wrapperFromClient);

	private:
		void handlePlayerSlot(Server& server, const ClientId& clientUuid, const tp_c2s::PlayerSlot& tpPlayerSlot);

		void handleGameCommand(Server& server, const tp_c2s::GameCommand& gameCommand);

		void handleStartGame(Server& server, const ClientId& clientUuid, const tp_c2s::StartGame& createServerGame);

		void handleBoardMove(Server& server, const ClientId& clientUuid, const tp_c2s::BoardMove& boardMove);

		void handleBoardNextBlock(Server& server, const ClientId& clientUuid, const tp_c2s::BoardNextBlock& boardNextBlock);

		void handleBoardExternalSquares(Server& server, const ClientId& clientUuid, const tp_c2s::BoardExternalSquares& boardExternalSquares);

		void handleRequestGameRestart(Server& server, const ClientId& clientUuid, const tp_c2s::RequestGameRestart& requestGameRestart);

		void handleGameRestart(Server& server, const ClientId& clientUuid, const tp_c2s::GameRestart& gameRestart);

		void handleCreateGameRoom(Server& server, const ClientId& clientUuid, const tp_c2s::CreateGameRoom& createGameRoom);

		void handleJoinGameRoom(Server& server, const ClientId& clientUuid, const tp_c2s::JoinGameRoom& joinGameRoom);

		void handleLeaveGameRoom(Server& server, const ClientId& clientUuid, const tp_c2s::LeaveGameRoom& leaveGameRoom);

		bool slotBelongsToClient(const ClientId& clientUuid, int slotIndex) const;

		std::string name_;
		GameRoomId gameRoomId_;

		std::vector<Slot> playerSlots_;
		std::vector<ClientId> connectedClientIds;
		bool paused_ = false;

		tp_s2c::Wrapper wrapperToClient_;
	};

}

#endif
