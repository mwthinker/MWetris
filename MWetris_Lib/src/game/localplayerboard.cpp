#include "localplayerboard.h"
#include "actionhandler.h"
#include "tetrisgameevent.h"

#include <helper.h>

#include <string>
#include <functional>

namespace mwetris::game {

	LocalPlayerBoard::LocalPlayerBoard(const tetris::TetrisBoard& board, const std::string& name)
		: PlayerBoard{board, name} {}

	void LocalPlayerBoard::update(Input input, double deltaTime) {
		// The time between each "gravity" move.
		double downTime = 1.0 / speed_;
		gravityMove_.setWaitingTime(downTime);

		gravityMove_.update(deltaTime, true);
		if (gravityMove_.doAction()) {
			updateTetrisBoard(tetris::Move::DownGravity);
		}

		leftHandler_.update(deltaTime, input.left && !input.right);
		if (leftHandler_.doAction()) {
			updateTetrisBoard(tetris::Move::Left);
		}

		rightHandler_.update(deltaTime, input.right && !input.left);
		if (rightHandler_.doAction()) {
			updateTetrisBoard(tetris::Move::Right);
		}

		downHandler_.update(deltaTime, input.down);
		if (downHandler_.doAction()) {
			updateTetrisBoard(tetris::Move::Down);
		}

		rotateHandler_.update(deltaTime, input.rotate);
		if (rotateHandler_.doAction()) {
			updateTetrisBoard(tetris::Move::RotateLeft);
		}

		downGroundHandler_.update(deltaTime, input.downGround);
		if (downGroundHandler_.doAction()) {
			updateTetrisBoard(tetris::Move::DownGround);
		}
	}

	void LocalPlayerBoard::addRow(int holes) {
		auto blockTypes = tetris::generateRow(tetrisBoard_.getColumns(), holes);
		externalRows_.insert(externalRows_.end(), blockTypes.begin(), blockTypes.end());
	}

	void LocalPlayerBoard::updateName(const std::string& name) {
		name_ = name;
	}

	void LocalPlayerBoard::updateClearedRows(int clearedRows) {
		clearedRows_ = clearedRows;
	}

	void LocalPlayerBoard::updateRestart() {
		externalRows_.clear();
		clearedRows_ = 0;
		restartTetrisBoard(tetris::randomBlockType(), tetris::randomBlockType());
	}

	void LocalPlayerBoard::updateGameOver() {
		updateTetrisBoard(tetris::Move::GameOver);
	}

	void LocalPlayerBoard::updatePlayerData(const PlayerData& playerData) {
		playerData_ = playerData;
	}

	void LocalPlayerBoard::handleBoardEvent(tetris::BoardEvent boardEvent, int value) {
		PlayerBoard::handleBoardEvent(boardEvent, value);
		if (boardEvent == tetris::BoardEvent::CurrentBlockUpdated) {
			setNextTetrisBlock(tetris::randomBlockType());

			leftHandler_.reset();
			rightHandler_.reset();
			downHandler_.reset();
		}
		if (boardEvent == tetris::BoardEvent::RowsRemoved) {
			clearedRows_ += value;
		}
		if (boardEvent == tetris::BoardEvent::BlockCollision) {
			tetrisBoard_.addExternalRows(externalRows_);
			externalRows_.clear();
		}
	}

}
