#ifndef APP_GAME_TETRISPARAMETERS_H
#define APP_GAME_TETRISPARAMETERS_H

#include <algorithm>

namespace app::game {

	constexpr int TetrisMaxLevel = 40;

	constexpr int TetrisMaxWidth = 40;
	constexpr int TetrisMaxHeight = 40;

	constexpr int TetrisMinWidth = 6;
	constexpr int TetrisMinHeight = 6;

	constexpr int TetrisWidth = 10;
	constexpr int TetrisHeight = 24;
	constexpr int RowsToLevelUp = 10;
	constexpr int CountDownTime = 3;

	constexpr int clampStartLevel(int level) {
		return std::clamp(level, 0, TetrisMaxLevel);
	}

	constexpr int rowsForFirstLevelUp(int startLevel) {
		startLevel = clampStartLevel(startLevel);
		return std::min((startLevel + 1) * RowsToLevelUp, std::max(100, startLevel * RowsToLevelUp - 50));
	}

	constexpr int levelForClearedRows(int clearedRows, int startLevel = 0) {
		startLevel = clampStartLevel(startLevel);
		const int firstLevelUp{rowsForFirstLevelUp(startLevel)};
		if (clearedRows < firstLevelUp) {
			return startLevel;
		}
		return startLevel + 1 + (clearedRows - firstLevelUp) / RowsToLevelUp;
	}

}

#endif
