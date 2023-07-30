#ifndef MWETRIS_GAME_SERIALIZE_H
#define MWETRIS_GAME_SERIALIZE_H

#include "localplayer.h"
#include "devicemanager.h"
#include "snapshot.h"

#include <chrono>

namespace mwetris::game {

	class DeviceManager;

	struct HighScoreResult {
		std::string name;
		int points = 0;
		int rows = 0;
		int level = 0;
		std::chrono::year_month_day lastPlayed;
	};

	bool isNewHighScore(const PlayerPtr& player);

	int getHighScorePlacement(int points);

	std::vector<HighScoreResult> loadHighScore();

	bool hasSavedGame();

	void clearSavedGame();

	void saveGame(const std::vector<PlayerDevice>& players);

	std::vector<PlayerDevice> loadGame(const DeviceManager& deviceManager);

	void saveHighScore(const std::string& name, int points, int rows, int level);

}

#endif
