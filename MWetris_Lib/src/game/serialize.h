#ifndef MWETRIS_GAME_SERIALIZE_H
#define MWETRIS_GAME_SERIALIZE_H

#include "localplayer.h"

#include <chrono>

namespace mwetris::game {

	struct HighScoreResult {
		std::string name;
		int points = 0;
		int rows = 0;
		int level = 0;
		std::chrono::year_month_day lastPlayed;
	};

	bool isNewHighScore(const PlayerPtr& player);

	int getHighscorePlacement(int points);

	std::vector<HighScoreResult> loadHighScore();

	bool hasSavedGame();

	void clearSavedGame();

	void saveGame(const std::vector<LocalPlayerPtr>& players);

	std::vector<LocalPlayerPtr> loadGame(const std::vector<DevicePtr>& availableDevices);

	void saveHighScore(const std::string& name, int points, int rows, int level);

}

#endif
