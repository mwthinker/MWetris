#ifndef APP_GAME_CLASSICSCORECOMPARISON_H
#define APP_GAME_CLASSICSCORECOMPARISON_H

#include "defaultgamerules.h"
#include "tetrisparameters.h"

#include <cmath>

namespace app::game {

	struct ClassicScoreEntry {
		int points;
		int level;
		int clearedRows;
		int startLevel;
	};

	struct ClassicScoreComparison {
		int pointDifference;
		double tetrisDifference;
	};

	inline ClassicScoreComparison calculateClassicScoreComparison(const ClassicScoreEntry& player, const ClassicScoreEntry& opponent) {
		const int pointDifference{player.points - opponent.points};
		if (pointDifference == 0) {
			return {};
		}

		const ClassicScoreEntry& trailingPlayer{pointDifference > 0 ? opponent : player};
		int remainingPoints{std::abs(pointDifference)};
		int clearedRows{trailingPlayer.clearedRows};
		int scoringLevel{trailingPlayer.level};
		double tetrisDifference{0.0};

		int tetrisPoints{calculateLineClearPoints(4, scoringLevel)};
		while (remainingPoints >= tetrisPoints) {
			remainingPoints -= tetrisPoints;
			tetrisDifference += 1.0;
			clearedRows += 4;
			scoringLevel = levelForClearedRows(clearedRows, trailingPlayer.startLevel);
			tetrisPoints = calculateLineClearPoints(4, scoringLevel);
		}
		tetrisDifference += static_cast<double>(remainingPoints) / tetrisPoints;

		return {
			.pointDifference = pointDifference,
			.tetrisDifference = pointDifference > 0 ? tetrisDifference : -tetrisDifference
		};
	}

}

#endif
