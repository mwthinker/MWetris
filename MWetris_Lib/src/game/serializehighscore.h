#ifndef MWETRIS_GAME_SERIALIZEHIGHSCORE_H
#define MWETRIS_GAME_SERIALIZEHIGHSCORE_H

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

	bool isNewHighScore(int points);

	int getHighScorePlacement(int points);

	std::vector<HighScoreResult> loadHighScore();

	void saveHighScore(const std::string& name, int points, int rows, int level);

}

#endif
