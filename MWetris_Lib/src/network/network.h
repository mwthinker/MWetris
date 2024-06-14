#ifndef MWETRIS_NETWORK_NETWORK_H
#define MWETRIS_NETWORK_NETWORK_H

#include "client.h"
#include "networkevent.h"
#include "id.h"

#include "game/playerslot.h"

#include <shared.pb.h>
#include <client_to_server.pb.h>
#include <server_to_client.pb.h>

#include <mw/signal.h>

#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <asio.hpp>

#include <chrono>

namespace mwetris::network {

	class Network : public std::enable_shared_from_this<Network> {
	public:
		mw::PublicSignal<Network, const PlayerSlotEvent&> playerSlotEvent;
		mw::PublicSignal<Network, const CreateGameRoomEvent&> createGameRoomEvent;
		mw::PublicSignal<Network, const JoinGameRoomEvent&> joinGameRoomEvent;
		mw::PublicSignal<Network, const RestartEvent&> restartEvent;
		mw::PublicSignal<Network, const PauseEvent&> pauseEvent;
		mw::PublicSignal<Network, const CreateGameEvent&> createGameEvent;
		mw::PublicSignal<Network, const LeaveGameRoomEvent&> leaveGameRoomEvent;

		struct NetworkPlayer {
			game::PlayerPtr player;
			PlayerId playerId;
			ClientId clientId;
		};

		explicit Network(std::shared_ptr<Client> client);

		void start();

		void stop();

		bool isInsideGameRoom() const;

		~Network();

		const GameRoomId& getGameRoomId() const;

		void startGame(int w, int h);

		void sendPause(bool pause);

		void sendRestart();

		void createGameRoom(const std::string& gameRoom);

		void leaveRoom();

		void setPlayerSlot(const game::PlayerSlot& playerSlot, int index);

		void joinGameRoom(const std::string& uuid);

		bool isInsideRoom() const;

	private:
		static asio::awaitable<void> run(std::shared_ptr<Network> network);

		static asio::awaitable<void> nextMessage(std::shared_ptr<Network> network);

		void handleRequestGameRestart(const tp_s2c::RequestGameRestart& requestGameRestart);

		void handleGameRestart(const tp_s2c::GameRestart& gameRestart);

		void handleGameCommand(const tp_s2c::GameCommand& gameCommand);

		void handlGameRoomCreated(const tp_s2c::GameRoomCreated& gameRoomCreated);

		void handleGameRoomJoined(const tp_s2c::GameRoomJoined& gameRoomJoined);

		void handleGameLooby(const tp_s2c::GameLooby& gameLooby);

		void handleConnections(const tp_s2c::Connections& connections);

		void handleCreateGame(int width, int height, const tp_s2c::CreateGame_Player& tpPlayer, const NetworkSlot& networkSlot);

		void handleCreateGame(const tp_s2c::CreateGame& createGame);

		void handleBoardMove(const tp_s2c::BoardMove& boardMove);

		void handleBoardNextBlock(const tp_s2c::BoardNextBlock& boardNextBlock);

		void handleBoardExternalSquares(const tp_s2c::BoardExternalSquares& boardExternalSquares);

		void fillSlotsWithDevicesAndAis();

		void handlePlayerBoardUpdate(const NetworkPlayer& player, const game::UpdateRestart& updateRestart);

		void handlePlayerBoardUpdate(const NetworkPlayer& player, const game::UpdatePlayerData& updatePlayerData);

		void handlePlayerBoardUpdate(const NetworkPlayer& player, const game::ExternalRows& externalRows);

		void handlePlayerBoardUpdate(const NetworkPlayer& player, const game::UpdateMove& updateMove);

		void handlePlayerBoardUpdate(const NetworkPlayer& player, const game::UpdateNextBlock& updateNextBlock);

		void handlePlayerBoardUpdate(const NetworkPlayer& player, const game::TetrisBoardEvent& tetrisBoardEvent);

		void send(const tp_c2s::Wrapper& wrapper);

		std::vector<NetworkSlot> networkSlots_;
		std::map<int, game::DevicePtr> deviceBySlotIndex_;
		std::map<int, tetris::Ai> aiBySlotIndex_;

		mw::signals::ScopedConnections connections_;

		std::vector<NetworkPlayer> players_;
		
		tp_c2s::Wrapper wrapperToServer_;
		tp_s2c::Wrapper wrapperFromServer_;
		std::shared_ptr<Client> client_;
		GameRoomId gameRoomId_;
		bool leaveRoom_ = false; // To make run() to proceed
		bool running_ = true;
		ClientId clientId_;
		asio::high_resolution_timer timer_;
	};

}

#endif
