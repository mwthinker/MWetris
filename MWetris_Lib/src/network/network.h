#ifndef MWETRIS_NETWORK_NETWORK_H
#define MWETRIS_NETWORK_NETWORK_H

#include "game/remoteplayerboard.h"

#include "game/remoteplayer.h"
#include "game/player.h"
#include "game/humanai.h"
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
		mw::PublicSignal<Network, game::PlayerSlot, int> playerSlotUpdate;

		Network(std::shared_ptr<Client> client);

		~Network();

		void update();

		void setPlayerSlot(const game::PlayerSlot& playerSlot, int slot);

		const std::string& getServerId() const;

		// Return true if ready.
		bool createGame(std::unique_ptr<game::GameRules> gameRules, int w, int h, game::TetrisGame& tetrisGame);

	private:
		class Impl;
		std::unique_ptr<Impl> impl_;
	};

}

#endif
