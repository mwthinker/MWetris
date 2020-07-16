#include "localplayerbuilder.h"

namespace tetris::game {

	LocalPlayerPtr LocalPlayerBuilder::build() {
		LocalPlayerPtr player;
		if (board_.size() > 0 || movingBlockType_ == BlockType::Empty) {
			TetrisBoard board(board_, width_, height_, movingBlock_, next_);
			player = std::make_shared<LocalPlayer>(eventManager_, board, device_);
		} else {
			TetrisBoard board(width_, height_, movingBlockType_, next_);
			player = std::make_shared<LocalPlayer>(eventManager_, board, device_);
		}
		player->updateName(name_);
		player->updateLevelUpCounter(levelUpCounter_);
		player->updateLevel(level_);
		//player->updateClearedRows(clearedRows_);
		player->updateGameOverPosition(gameOverPosition_);
		player->updatePoints(points_);
		return player;
	}

}
