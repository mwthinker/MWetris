#include "localplayer.h"
#include "actionhandler.h"
#include "tetrisgameevent.h"

#include <helper.h>

#include <string>
#include <functional>

namespace mwetris::game {

	LocalPlayer::~LocalPlayer() {
	}

	LocalPlayer::LocalPlayer(const tetris::TetrisBoard& board, const DevicePtr& device)
		: Player{board}
		, leftHandler_{0.09, false}
		, rightHandler_{0.09, false}
		, rotateHandler_{0.0, true}
		, downGroundHandler_{0.0, true}
		, gravityMove_{1, false}  // Value doesn't matter! Changes every frame.
		, downHandler_{0.04, false}
		, device_{device}
		, levelUpCounter_{0} {
		
		device_->update(getTetrisBoard());
		name_ = device_->getName();
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

		device_->update(getTetrisBoard());
	}

	void LocalPlayer::addRow(int holes) {
		//auto blockTypes = tetris::generateRow(tetrisBoard_.getColumns(), 2);
		//tetrisBoard_.addRows(blockTypes);
	}

	void LocalPlayer::updateName(const std::string& name) {
		name_ = name;
	}

	void LocalPlayer::updatePoints(int points) {
		points_ = points;
	}

	void LocalPlayer::updateLevelUpCounter(int counter) {
		levelUpCounter_ = counter;
	}

	void LocalPlayer::updateLevel(int level) {
		level_ = level;
	}

	void LocalPlayer::updateGameOverPosition(int gameOverPosition) {
		gameOverPosition_ = gameOverPosition;
	}

	void LocalPlayer::updateRestart() {
		level_ = 1;
		points_ = 0;
		gameOverPosition_ = 0;
		levelUpCounter_ = 0;
		restartTetrisBoard(tetris::randomBlockType(), tetris::randomBlockType());
	}

	void LocalPlayer::updateGameOver() {
		updateTetrisBoard(tetris::Move::GameOver);
	}

	void LocalPlayer::boardListener(tetris::BoardEvent gameEvent) {
		if (gameEvent == tetris::BoardEvent::CurrentBlockUpdated) {
			setNextTetrisBlock(tetris::randomBlockType());

			leftHandler_.reset();
			rightHandler_.reset();
			downHandler_.reset();
		}
	}

}
