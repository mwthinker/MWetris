#include "localplayerboardbuilder.h"
#include "tetrisboard.h"

#include <memory>

namespace mwetris::game {

	LocalPlayerBoardPtr LocalPlayerBoardBuilder::build() {
		LocalPlayerBoardPtr playerBoard;
		if (board_.size() > 0 || movingBlockType_ == tetris::BlockType::Empty) {
			tetris::TetrisBoard board(board_, width_, height_, movingBlock_, next_);
			playerBoard = std::make_shared<LocalPlayerBoard>(board, name_);
		} else {
			tetris::TetrisBoard board(width_, height_, movingBlockType_, next_);
			playerBoard = std::make_shared<LocalPlayerBoard>(board, name_);
		}
		playerBoard->updateLevel(level_);
		playerBoard->updateClearedRows(clearedRows_);
		playerBoard->updateGameOverPosition(gameOverPosition_);
		playerBoard->updatePoints(points_);
		return playerBoard;
	}

}
