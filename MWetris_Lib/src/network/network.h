#ifndef MWETRIS_NETWORK_NETWORK_H
#define MWETRIS_NETWORK_NETWORK_H

#include "game/remoteplayerboard.h"

#include "game/remoteplayer.h"
#include "game/player.h"
#include "game/playerslot.h"
#include "game/defaultgamerules.h"
#include "protobufmessage.h"
#include "client.h"

#include <thread>
#include <memory>
#include <string>

#include <mw/signal.h>

namespace mwetris::game {

	class TetrisGame;

}

namespace mwetris::network {

	class Network {
	public:
		Network(std::shared_ptr<Client> client, std::shared_ptr<game::TetrisGame> tetrisGame);

		~Network();

		void update();

		void setPlayerSlot(const game::PlayerSlot& playerSlot, int slot);

		const std::string& getGameRoomUuid() const;

		void sendPause(bool pause);

		void joinGameRoom(const std::string& serverId);

		void createGameRoom(const std::string& name);

		void leaveRoom();

		bool isInsideRoom() const;

		// Return true if ready.
		bool startGame(std::unique_ptr<game::GameRules> gameRules, int w, int h);

		mw::signals::Connection addPlayerSlotListener(std::function<void(game::PlayerSlot, int)> listener);

	private:
		class Impl;
		std::unique_ptr<Impl> impl_;
	};

}

#endif
