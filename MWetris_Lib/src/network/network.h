#ifndef MWETRIS_NETWORK_NETWORK_H
#define MWETRIS_NETWORK_NETWORK_H

#include "protocol.h"
#include "game/aiplayer.h"
#include "game/playerslot.h"
#include "network/client.h"
#include "network/networkevent.h"

#include <shared.pb.h>
#include <client_to_server.pb.h>
#include <server_to_client.pb.h>

#include <net/client.h>

#include <mw/signal.h>

#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <concurrencpp/concurrencpp.h>

namespace conc = concurrencpp;

namespace mwetris::network {

	class Network {
	public:
		mw::PublicSignal<Network, const PlayerSlotEvent&> playerSlotEvent;
		mw::PublicSignal<Network, const CreateGameRoomEvent&> createGameRoomEvent;
		mw::PublicSignal<Network, const JoinGameRoomEvent&> joinGameRoomEvent;
		mw::PublicSignal<Network, const RestartEvent&> restartEvent;
		mw::PublicSignal<Network, const PauseEvent&> pauseEvent;
		mw::PublicSignal<Network, const CreateGameEvent&> createGameEvent;

		struct NetworkPlayer {
			game::PlayerPtr player;
			std::string uuid;
		};

		Network(std::shared_ptr<Client> client);

		bool isActive() const;

		~Network();

		const std::string& getGameRoomUuid() const;

		void startGame(int w, int h);

		void update();

		void sendPause(bool pause);

		void sendRestart();

		void createGameRoom(const std::string& gameRoom);

		void leaveRoom();

		void setPlayerSlot(const game::PlayerSlot& playerSlot, int index);

		void joinGameRoom(const std::string& uuid);

		bool isInsideRoom() const;

	private:
		conc::result<void> stepOnce();

		void handleConnection();

		conc::result<void> nextMessage();

		void handleGameRestart(const tp_s2c::GameRestart& gameRestart);

		void handleGameCommand(const tp_s2c::GameCommand& gameCommand);

		void handlGameRoomCreated(const tp_s2c::GameRoomCreated& gameRoomCreated);

		void handleGameRoomJoined(const tp_s2c::GameRoomJoined& gameRoomJoined);

		void handleGameLooby(const tp_s2c::GameLooby& gameLooby);

		void handleConnections(const tp_s2c::Connections& connections);

		void handleCreateGame(int width, int height, const tp_s2c::CreateGame_Player& tpPlayer, const NetworkSlot& networkSlot);

		void handleCreateGame(const tp_s2c::CreateGame& createGame);

		void fillSlotsWithDevicesAndAis();

		void restartGame(game::GameRestart gameRestart);

		void handlePlayerBoardUpdate(const NetworkPlayer& player, const game::UpdateRestart& updateRestart);

		void handlePlayerBoardUpdate(const NetworkPlayer& player, const game::UpdatePlayerData& updatePlayerData);

		void handlePlayerBoardUpdate(const NetworkPlayer& player, const game::ExternalRows& externalRows);

		void handlePlayerBoardUpdate(const NetworkPlayer& player, const game::UpdateMove& updateMove);

		void handlePlayerBoardUpdate(const NetworkPlayer& player, const game::UpdateNextBlock& updateNextBlock);

		void handleBoardEvent(const game::Player& player, tetris::BoardEvent boardEvent, int nbr);

		void send(const tp_c2s::Wrapper& wrapper);

		conc::runtime runtime_;
		std::shared_ptr<conc::manual_executor> manualExecutor_;
		conc::async_lock lock_;
		conc::async_condition_variable cv_;

		std::vector<NetworkSlot> networkSlots_;
		std::map<int, game::DevicePtr> deviceBySlotIndex_;
		std::map<int, tetris::Ai> aiBySlotIndex_;

		mw::signals::ScopedConnections connections_;

		std::vector<NetworkPlayer> players_;
		
		tp_c2s::Wrapper wrapperToServer_;
		tp_s2c::Wrapper wrapperFromServer_;
		std::shared_ptr<Client> client_;
		std::string gameRoomUuid_;
		std::string clientUuid_;
	};

}

#endif
