#include "localplayer.h"
#include "actionhandler.h"
#include "tetrisboardwrapper.h"

#include <string>
#include <functional>

namespace tetris::game {

	LocalPlayer::~LocalPlayer() {
		connection_.disconnect();
	}


	LocalPlayer::LocalPlayer(const TetrisBoard& board, const DevicePtr& device)
		: leftHandler_{0.09, false}
		, rightHandler_{0.09, false}
		, rotateHandler_{0.0, true}
		, downGroundHandler_{0.0, true}
		, gravityMove_{1, false}  // Value doesn't matter! Changes every frame.
		, downHandler_{0.04, false}
		, device_{device}
		, levelUpCounter_{0}
		, tetrisBoard_{board.getBoardVector(), board.getColumns(), board.getRows(), board.getBlock(), board.getNextBlockType()} {

		connection_ = tetrisBoard_.addGameEventListener([&](BoardEvent gameEvent, const TetrisBoardWrapper& board) {
			boardListener(gameEvent);
		});
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
			tetrisBoard_.update(Move::DownGravity);
		}

		leftHandler_.update(deltaTime, input.left && !input.right);
		if (leftHandler_.doAction()) {
			tetrisBoard_.update(Move::Left);
		}

		rightHandler_.update(deltaTime, input.right && !input.left);
		if (rightHandler_.doAction()) {
			tetrisBoard_.update(Move::Right);
		}

		downHandler_.update(deltaTime, input.down);
		if (downHandler_.doAction()) {
			tetrisBoard_.update(Move::Down);
		}

		rotateHandler_.update(deltaTime, input.rotate);
		if (rotateHandler_.doAction()) {
			tetrisBoard_.update(Move::RotateLeft);
		}

		downGroundHandler_.update(deltaTime, input.downGround);
		if (downGroundHandler_.doAction()) {
			tetrisBoard_.update(Move::DownGround);
		}

		device_->update(tetrisBoard_);
	}

	void LocalPlayer::addRow(int holes) {
		std::vector<BlockType> blockTypes = generateRow(tetrisBoard_.getColumns(), 2);
		tetrisBoard_.addRows(blockTypes);
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
		tetrisBoard_.restart(randomBlockType(), randomBlockType());
	}

	void LocalPlayer::updateGameOver() {
		tetrisBoard_.update(Move::GameOver);
	}

	void LocalPlayer::boardListener(BoardEvent gameEvent) {
		if (gameEvent == BoardEvent::CurrentBlockUpdated) {
			tetrisBoard_.setNextBlock(randomBlockType());

			leftHandler_.reset();
			rightHandler_.reset();
			downHandler_.reset();
		}
	}

}
