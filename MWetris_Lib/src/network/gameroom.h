#ifndef MWETRIS_NETWORK_GAMEROOM_H
#define MWETRIS_NETWORK_GAMEROOM_H

#include "debugserver.h"
#include "game/remoteplayer.h"
#include "server.h"

#include <server_to_client.pb.h>
#include <client_to_server.pb.h>

#include <map>
#include <string>
#include <vector>

namespace mwetris::network {

	class GameRoom {
	public:
		GameRoom() = default;

		GameRoom(const std::string& name);

		~GameRoom();

		const std::string& getName() const;

		const std::string& getMasterUuid() const;

		void connectMaster(Server& server);

		void disconnect(Server& server, const std::string& uuid);

		void sendPause(Server& server, bool pause);

		bool isPaused() const;

		void restartGame(Server& server);

		void receiveMessage(Server& server, const std::string& clientUuid, const tp_c2s::Wrapper& wrapperFromClient);

	private:
		void handleConnect(Server& server, const std::string& clientUuid, const tp_c2s::Connect& connect);

		void handleGameLooby(const std::string& clientUuid, const tp_c2s::GameLooby& gameLooby);

		void handlePlayerSlot(Server& server, const std::string& clientUuid, const tp_c2s::PlayerSlot& tpPlayerSlot);

		void handleGameCommand(Server& server, const tp_c2s::GameCommand& gameCommand);

		void handleStartGame(Server& server, const std::string& clientUuid, const tp_c2s::StartGame& createServerGame);

		void handleBoardMove(Server& server, const std::string& clientUuid, const tp_c2s::BoardMove& boardMove);

		void handleBoardNextBlock(Server& server, const std::string& clientUuid, const tp_c2s::BoardNextBlock& boardNextBlock);

		void handleBoardExternalSquares(Server& server, const std::string& clientUuid, const tp_c2s::BoardExternalSquares& boardExternalSquares);

		void handleGameRestart(Server& server, const std::string& clientUuid, const tp_c2s::GameRestart& gameRestart);

		void handleConnectToGame(Server& server, const std::string& clientUuid, const tp_c2s::ConnectToGame& connectToGame);
	
		bool slotBelongsToClient(const std::string& clientUuid, int slotIndex) const;

		std::string name_;
		std::string masterUuid_;

		std::map<std::string, game::RemotePlayerPtr> remotePlayers_;
		std::vector<Slot> playerSlots_;
		std::vector<std::string> connectedUuids_;
		bool paused_ = false;

		tp_s2c::Wrapper wrapperToClient_;
	};

}

#endif
