#include "player.h"
#include "tetrisgameevent.h"
#include "helper.h"

namespace mwetris::game {

	bool Player::isGameOver() const {
		return tetrisBoard_.isGameOver();
	}

	void Player::restartTetrisBoard(tetris::BlockType current, tetris::BlockType next) {
		tetrisBoard_.restart(tetris::randomBlockType(), tetris::randomBlockType());
	}

	void Player::setNextTetrisBlock(tetris::BlockType next) {
		tetrisBoard_.setNextBlock(next);
	}

	void Player::updateTetrisBoard(tetris::Move move) {
		tetrisBoard_.update(move, [this](tetris::BoardEvent boardEvent, int value) {
			handleBoardEvent(boardEvent, value);
		});
	}

	void Player::handleBoardEvent(tetris::BoardEvent event, int value) {
		gameboardEventUpdate.invoke(event, value);
	}

}
