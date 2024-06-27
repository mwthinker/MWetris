#ifndef MWETRIS_GAME_GAMERULES_H
#define MWETRIS_GAME_GAMERULES_H

#include "player.h"

#include <vector>

namespace mwetris::game {

	class GameRules {
	public:
		virtual ~GameRules() = default;

		virtual void update(double deltaTime) = 0;

		virtual void restart() = 0;

		virtual void createGame(const std::vector<PlayerPtr>& players) = 0;
	};

}

#endif
