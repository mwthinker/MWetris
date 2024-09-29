#ifndef APP_GAME_GAMERULES_H
#define APP_GAME_GAMERULES_H

#include "player.h"

#include <vector>

namespace app::game {

	class GameRules {
	public:
		virtual ~GameRules() = default;

		virtual void restart() = 0;

		virtual void createGame(const std::vector<PlayerPtr>& players) = 0;
	};

}

#endif
