#ifndef MWETRIS_GAME_LOCALGAME_H
#define MWETRIS_GAME_LOCALGAME_H

#include "player.h"
#include "localplayer.h"
#include "tetrisgameevent.h"

namespace mwetris::game {

	class LocalGame {
	public:
		LocalGame();

		void createGame(LocalPlayerPtr player);

		void restartGame();

		bool isPaused() const {
			return false;
		}

		void setPaused(bool pause) {
			pause_ = pause;
		}

	private:
		void applyRules(tetris::BoardEvent gameEvent, int value, const LocalPlayerPtr& player);

		LocalPlayerPtr player_;
		
		int points_ = 0;
		int rows_ = 0;
		int level_ = 1;
		bool pause_ = false;
		mw::signals::ScopedConnection connection_;
	};

}

#endif
