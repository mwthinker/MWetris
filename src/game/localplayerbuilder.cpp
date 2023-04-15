#include "localplayerbuilder.h"

namespace mwetris::game {

	LocalPlayerPtr LocalPlayerBuilder::build() {
		LocalPlayerPtr player;
		if (board_.size() > 0 || movingBlockType_ == tetris::BlockType::Empty) {
			tetris::TetrisBoard board(board_, width_, height_, movingBlock_, next_);
			player = std::make_shared<LocalPlayer>(board, device_);
		} else {
			tetris::TetrisBoard board(width_, height_, movingBlockType_, next_);
			player = std::make_shared<LocalPlayer>(board, device_);
		}
		player->updateName(name_);
		player->updateLevel(level_);
		player->updateClearedRows(clearedRows_);
		player->updateGameOverPosition(gameOverPosition_);
		player->updatePoints(points_);
		return player;
	}

}
