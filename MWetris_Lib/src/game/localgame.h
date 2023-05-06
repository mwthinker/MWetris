#ifndef MWETRIS_GAME_LOCALGAME_H
#define MWETRIS_GAME_LOCALGAME_H

#include "player.h"
#include "localplayer.h"
#include "tetrisgameevent.h"

namespace mwetris::game {

	class LocalGame {
	public:
		LocalGame();

		void resume(const std::vector<LocalPlayerPtr>& players);

		[[nodiscard]] std::vector<LocalPlayerPtr> create(int columns, int rows, const std::vector<DevicePtr>& devices);

		void restart();

	private:
		void applyRules(tetris::BoardEvent gameEvent, int value, const LocalPlayerPtr& player);
		
		std::vector<LocalPlayerPtr> players_;
		
		int points_ = 0;
		int rows_ = 0;
		int level_ = 1;
		mw::signals::ScopedConnections connections_;
	};

}

#endif
