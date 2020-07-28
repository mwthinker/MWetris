#include "localplayer.h"
#include "actionhandler.h"
#include "tetrisboardwrapper.h"
#include "tetrisgameevent.h"

#include <string>
#include <functional>

namespace mwetris::game {

	LocalPlayer::~LocalPlayer() {
	}

	LocalPlayer::LocalPlayer(std::shared_ptr<EventManager> eventManager, const tetris::TetrisBoard& board, const DevicePtr& device)
		: Player{eventManager}
		, leftHandler_{0.09, false}
		, rightHandler_{0.09, false}
		, rotateHandler_{0.0, true}
		, downGroundHandler_{0.0, true}
		, gravityMove_{1, false}  // Value doesn't matter! Changes every frame.
		, downHandler_{0.04, false}
		, device_{device}
		, levelUpCounter_{0}
		, tetrisBoard_{tetris::TetrisBoard{board.getBoardVector(), board.getColumns(), board.getRows(), board.getBlock(), board.getNextBlockType()}} {
		
		//SubscriptionHandle handle = eventManager_->subscribe(getSenderId(), [](std::shared_ptr<Event> event) {
		//});
		
		//eventManager_->publish<TestEvent>(senderId_, 5);

		//eventManager_->unsubscribe(handle);

		/*
		connection_ = tetrisBoard_.addGameEventListener([&](BoardEvent gameEvent, const TetrisBoardWrapper& board) {
			boardListener(gameEvent);
		});
		*/

		device_->update(getTetrisBoard());
		name_ = device_->getName();
	}

	void LocalPlayer::update(double deltaTime) {
		Input input = device_->getInput();

		// The time beetween each "gravity" move.
		double downTime = 1.0 / getGravityDownSpeed();
		gravityMove_.setWaitingTime(downTime);

		auto callback = [this](TetrisBoardWrapper::Event event, int value) {
			switch (event) {
				case TetrisBoardWrapper::Event::GameOver:
					publishEvent<GameOver>(shared_from_this());
					break;
				case TetrisBoardWrapper::Event::BlockCollision:
					publishEvent<GameOver>(shared_from_this());
					break;
			}
		};

		gravityMove_.update(deltaTime, true);
		if (gravityMove_.doAction()) {
			tetrisBoard_.update(tetris::Move::DownGravity, callback);
		}

		leftHandler_.update(deltaTime, input.left && !input.right);
		if (leftHandler_.doAction()) {
			tetrisBoard_.update(tetris::Move::Left, callback);
		}

		rightHandler_.update(deltaTime, input.right && !input.left);
		if (rightHandler_.doAction()) {
			tetrisBoard_.update(tetris::Move::Right, callback);
		}

		downHandler_.update(deltaTime, input.down);
		if (downHandler_.doAction()) {
			tetrisBoard_.update(tetris::Move::Down, callback);
		}

		rotateHandler_.update(deltaTime, input.rotate);
		if (rotateHandler_.doAction()) {
			tetrisBoard_.update(tetris::Move::RotateLeft, callback);
		}

		downGroundHandler_.update(deltaTime, input.downGround);
		if (downGroundHandler_.doAction()) {
			tetrisBoard_.update(tetris::Move::DownGround, callback);
		}

		device_->update(tetrisBoard_);
	}

	void LocalPlayer::addRow(int holes) {
		auto blockTypes = tetris::generateRow(tetrisBoard_.getColumns(), 2);
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
		tetrisBoard_.restart(tetris::randomBlockType(), tetris::randomBlockType());
	}

	void LocalPlayer::updateGameOver() {
		tetrisBoard_.update(tetris::Move::GameOver, [](TetrisBoardWrapper::Event event, int value) {

		});
	}

	void LocalPlayer::boardListener(tetris::BoardEvent gameEvent) {
		if (gameEvent == tetris::BoardEvent::CurrentBlockUpdated) {
			tetrisBoard_.setNextBlock(tetris::randomBlockType());

			leftHandler_.reset();
			rightHandler_.reset();
			downHandler_.reset();
		}
	}

}
