#include "localplayer.h"
#include "actionhandler.h"
#include "tetrisgameevent.h"

#include <helper.h>

#include <string>
#include <functional>

namespace mwetris::game {

	LocalPlayer::LocalPlayer(const tetris::TetrisBoard& board, DevicePtr device)
		: Player{board}
		, device_{std::move(device)}
		, name_{device_->getName()} {
	}

	void LocalPlayer::update(double deltaTime) {
		Input input = device_->getInput();

		// The time beetween each "gravity" move.
		double downTime = 1.0 / getGravityDownSpeed();
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

	void LocalPlayer::addRow(int holes) {
		auto blockTypes = tetris::generateRow(tetrisBoard_.getColumns(), holes);
		tetrisBoard_.addExternalRows(blockTypes);
	}

	void LocalPlayer::updateName(const std::string& name) {
		name_ = name;
	}

	void LocalPlayer::updatePoints(int points) {
		points_ = points;
	}

	void LocalPlayer::updateLevel(int level) {
		level_ = level;
	}

	void LocalPlayer::updateClearedRows(int clearedRows) {
		clearedRows_ = clearedRows;
	}

	void LocalPlayer::updateGameOverPosition(int gameOverPosition) {
		gameOverPosition_ = gameOverPosition;
	}

	void LocalPlayer::updateRestart() {
		level_ = 1;
		points_ = 0;
		gameOverPosition_ = 0;
		clearedRows_ = 0;
		restartTetrisBoard(tetris::randomBlockType(), tetris::randomBlockType());
	}

	void LocalPlayer::updateGameOver() {
		updateTetrisBoard(tetris::Move::GameOver);
	}

	void LocalPlayer::handleBoardEvent(tetris::BoardEvent boardEvent, int value) {
		device_->onGameboardEvent(tetrisBoard_, boardEvent, value);

		Player::handleBoardEvent(boardEvent, value);
		if (boardEvent == tetris::BoardEvent::CurrentBlockUpdated) {
			setNextTetrisBlock(tetris::randomBlockType());

			leftHandler_.reset();
			rightHandler_.reset();
			downHandler_.reset();
		}
		if (boardEvent == tetris::BoardEvent::RowsRemoved) {
			clearedRows_ += value;
		}
	}

}
