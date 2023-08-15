#ifndef MWETRIS_GAME_DEFAULTGAMERULES_H
#define MWETRIS_GAME_DEFAULTGAMERULES_H

#include "tetrisboard.h"
#include "localplayerboard.h"

namespace mwetris::game {

	constexpr int LevelUpNbr = 1;

	class DefaultGameRules {
	public:
		DefaultGameRules() {
			restart();
		}

		void updateGameBoardEvent(tetris::BoardEvent gameEvent, int value, const LocalPlayerBoardPtr& playerBoard) {
			if (gameEvent == tetris::BoardEvent::RowsRemoved) {
				rows_ += value;
				level_ = rows_ / LevelUpNbr + 1;
				points_ += level_ * value * value;

				playerBoard->updateLevel(level_);
				playerBoard->updatePoints(points_);
			}
		}

		void restart() {
			rows_ = 0;
			level_ = 1;
			points_ = 0;
		}

	private:
		int rows_;
		int level_;
		int points_;
	};

}

#endif
