#ifndef APP_NETWORK_NETWORK_H
#define APP_NETWORK_NETWORK_H

#include "networkevent.h"
#include "../game/playerslot.h"

#include <network/client.h>
#include <network/id.h>

#include <protocol/shared.pb.h>
#include <protocol/client_to_server.pb.h>
#include <protocol/server_to_client.pb.h>

#include <mw/signal.h>

#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <asio.hpp>

#include <chrono>

namespace app::cnetwork {

	class Network : public std::enable_shared_from_this<Network> {
	public:
		mw::PublicSignal<Network, const NetworkEvent&> networkEvent;

		struct NetworkPlayer {
			game::PlayerPtr player;
			network::PlayerId playerId;
			network::ClientId clientId;
			bool connected = true;
		};

		explicit Network(std::shared_ptr<network::Client> client);

		void start();

		void stop();

		bool isPublic() const;

		bool isInsideGameRoom() const;

		~Network();

		const network::GameRoomId& getGameRoomId() const;

		void startGame(const game::GameRulesConfig& gameRulesConfig);

		void sendPause(bool pause);

		void sendRestart();

		void createGameRoom(const std::string& gameRoom, bool isPublic);

		void leaveGameRoom();

		void setPlayerSlot(const game::PlayerSlot& playerSlot, int index);

		void joinGameRoom(const network::GameRoomId& gameRoomId);

		bool isInsideRoom() const;

		void removeClient(const network::ClientId& clientId);

		void requestGameRoomList();

		bool isConnected() const;

	private:
		void reconnect();

		static asio::awaitable<void> run(std::shared_ptr<Network> network);

		static asio::awaitable<void> nextMessage(std::shared_ptr<Network> network);

		void handleRequestGameRestart(const tp_s2c::RequestGameRestart& requestGameRestart);

		void handleGameRestart(const tp_s2c::GameRestart& gameRestart);

		void handleGameCommand(const tp_s2c::GameCommand& gameCommand);

		void handleGameRoomJoined(const tp_s2c::GameRoomJoined& gameRoomJoined);

		void handleGameLooby(const tp_s2c::GameLooby& gameLooby);

		void handleConnections(const tp_s2c::Connections& connections);

		void handleCreateGame(int width, int height, const tp_s2c::CreateGame_Player& tpPlayer, const NetworkSlot& networkSlot);

		void handleCreateGame(const tp_s2c::CreateGame& createGame);

		void handleBoardMove(const tp_s2c::BoardMove& boardMove);

		void handleBoardNextBlock(const tp_s2c::BoardNextBlock& boardNextBlock);

		void handleBoardExternalSquares(const tp_s2c::BoardExternalSquares& boardExternalSquares);

		void handleClientDisconnected(const tp_s2c::ClientDisconnected& clientDisconnected);

		void handleRemoveClient(const tp_s2c::RemoveClient& removeClient);

		void fillSlotsWithDevicesAndAis();

		void handlePlayerBoardUpdate(const NetworkPlayer& player, const game::UpdateRestart& updateRestart);

		void handlePlayerBoardUpdate(const NetworkPlayer& player, const game::UpdatePlayerData& updatePlayerData);

		void handlePlayerBoardUpdate(const NetworkPlayer& player, const game::ExternalRows& externalRows);

		void handlePlayerBoardUpdate(const NetworkPlayer& player, const game::UpdateMove& updateMove);

		void handlePlayerBoardUpdate(const NetworkPlayer& player, const game::UpdateNextBlock& updateNextBlock);

		void handlePlayerBoardUpdate(const NetworkPlayer& player, const game::TetrisBoardEvent& tetrisBoardEvent);

		void handleLeaveGameRoom(const tp_s2c::LeaveGameRoom& leaveGameRoom);

		void handleGameRoomList(const tp_s2c::GameRoomList& gameRoomList);

		void send(const tp_c2s::Wrapper& wrapper);

		std::vector<NetworkSlot> networkSlots_;
		std::map<int, game::DevicePtr> deviceBySlotIndex_;
		std::map<int, tetris::Ai> aiBySlotIndex_;

		mw::signals::ScopedConnections connections_;

		std::vector<NetworkPlayer> players_;
		
		tp_c2s::Wrapper wrapperToServer_;
		tp_s2c::Wrapper wrapperFromServer_;
		std::shared_ptr<network::Client> client_;
		network::GameRoomId gameRoomId_;
		bool running_ = true;
		network::ClientId clientId_;
		asio::high_resolution_timer timer_;
		bool public_ = false;
	};

}

#endif
