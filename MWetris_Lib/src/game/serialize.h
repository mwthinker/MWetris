#ifndef MWETRIS_GAME_SERIALIZE_H
#define MWETRIS_GAME_SERIALIZE_H

#include "localplayerboard.h"

#include <chrono>

namespace mwetris::game {

	struct HighScoreResult {
		std::string name;
		int points = 0;
		int rows = 0;
		int level = 0;
		std::chrono::year_month_day lastPlayed;
	};

	bool isNewHighScore(const PlayerBoard& playerBoard);

	int getHighScorePlacement(int points);

	std::vector<HighScoreResult> loadHighScore();

	bool hasSavedGame();

	void clearSavedGame();

	void saveGame(const PlayerBoard& playerBoard);

	LocalPlayerBoardPtr loadGame();

	void saveHighScore(const std::string& name, int points, int rows, int level);

}

#endif
