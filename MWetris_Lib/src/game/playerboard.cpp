#include "playerboard.h"
#include "tetrisgameevent.h"

namespace mwetris::game {

	PlayerBoard::PlayerBoard(const tetris::TetrisBoard& tetrisBoard, const std::string& name)
		: tetrisBoard_{tetrisBoard}
		, name_{name} {}

	bool PlayerBoard::isGameOver() const {
		return tetrisBoard_.isGameOver();
	}

	void PlayerBoard::restartTetrisBoard(tetris::BlockType current, tetris::BlockType next) {
		tetrisBoard_.restart(current, next);
	}

	void PlayerBoard::setNextTetrisBlock(tetris::BlockType next) {
		tetrisBoard_.setNextBlock(next);
	}

	void PlayerBoard::updateTetrisBoard(tetris::Move move) {
		tetrisBoard_.update(move, [this](tetris::BoardEvent boardEvent, int value) {
			handleBoardEvent(boardEvent, value);
		});
	}

	void PlayerBoard::handleBoardEvent(tetris::BoardEvent event, int value) {
		gameboardEventUpdate.invoke(event, value);
	}

}
