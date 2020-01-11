#include "localplayer.h"
#include "actionhandler.h"
#include "tetrisboard.h"

#include <string>
#include <functional>

using namespace tetris;

ILocalPlayer::~ILocalPlayer() {
	connection_.disconnect();
}

ILocalPlayerPtr LocalPlayerBuilder::build() {
	ILocalPlayerPtr player;
	if (board_.size() > 0 || movingBlockType_ == BlockType::EMPTY) {
		RawTetrisBoard board(board_, width_, height_, movingBlock_, next_);
		player = std::make_shared<ILocalPlayer>(board, device_);
	} else {
		RawTetrisBoard board(width_, height_, movingBlockType_, next_);
		player = std::make_shared<ILocalPlayer>(board, device_);
	}
	player->updateName(name_);
	player->updateLevelUpCounter(levelUpCounter_);
	player->updateLevel(level_);
	//player->updateClearedRows(clearedRows_);
	player->updateGameOverPosition(gameOverPosition_);
	player->updatePoints(points_);
	return player;
}

ILocalPlayer::ILocalPlayer(const RawTetrisBoard& board, const IDevicePtr& device)
	: leftHandler_{0.09, false}
	, rightHandler_{0.09, false}
	, rotateHandler_{0.0, true}
	, downGroundHandler_{0.0, true}
	, gravityMove_{1, false}  // Value doesn't matter! Changes every frame.
	, downHandler_{0.04, false}
	, device_{device}
	, levelUpCounter_{0}
	, tetrisBoard_{board.getBoardVector(), board.getColumns(), board.getRows(), board.getBlock(), board.getNextBlockType()} {
	
	connection_ = tetrisBoard_.addGameEventListener([&](BoardEvent gameEvent, const TetrisBoard& board) {
		boardListener(gameEvent);
	});
	device_->update(getTetrisBoard());
	name_ = device_->getName();
}

void ILocalPlayer::update(double deltaTime) {
	Input input = device_->getInput();

	// The time beetween each "gravity" move.
	double downTime = 1.0 / getGravityDownSpeed();
	gravityMove_.setWaitingTime(downTime);

	gravityMove_.update(deltaTime, true);
	if (gravityMove_.doAction()) {
		tetrisBoard_.update(Move::DOWN_GRAVITY);
	}

	leftHandler_.update(deltaTime, input.left_ && !input.right_);
	if (leftHandler_.doAction()) {
		tetrisBoard_.update(Move::LEFT);
	}

	rightHandler_.update(deltaTime, input.right_ && !input.left_);
	if (rightHandler_.doAction()) {
		tetrisBoard_.update(Move::RIGHT);
	}

	downHandler_.update(deltaTime, input.down_);
	if (downHandler_.doAction()) {
		tetrisBoard_.update(Move::DOWN);
	}

	rotateHandler_.update(deltaTime, input.rotate_);
	if (rotateHandler_.doAction()) {
		tetrisBoard_.update(Move::ROTATE_LEFT);
	}

	downGroundHandler_.update(deltaTime, input.downGround_);
	if (downGroundHandler_.doAction()) {
		tetrisBoard_.update(Move::DOWN_GROUND);
	}

	device_->update(tetrisBoard_);
}

void ILocalPlayer::addRow(int holes) {
	std::vector<BlockType> blockTypes = generateRow(tetrisBoard_.getColumns(), 2);
	tetrisBoard_.addRows(blockTypes);
}

void ILocalPlayer::updateName(const std::string& name) {
	name_ = name;
}

void ILocalPlayer::updatePoints(int points) {
	points_ = points;
}

void ILocalPlayer::updateLevelUpCounter(int counter) {
	levelUpCounter_ = counter;
}

void ILocalPlayer::updateLevel(int level) {
	level_ = level;
}

void ILocalPlayer::updateGameOverPosition(int gameOverPosition) {
	gameOverPosition_ = gameOverPosition;
}

void ILocalPlayer::updateRestart() {
	level_ = 1;
	points_ = 0;
	gameOverPosition_ = 0;
	levelUpCounter_ = 0;
	tetrisBoard_.updateRestart(randomBlockType(), randomBlockType());
}

void ILocalPlayer::updateGameOver() {
	tetrisBoard_.update(Move::GAME_OVER);
}

void ILocalPlayer::boardListener(BoardEvent gameEvent) {
	if (gameEvent == BoardEvent::CURRENT_BLOCK_UPDATED) {
		tetrisBoard_.updateNextBlock(randomBlockType());

		leftHandler_.reset();
		rightHandler_.reset();
		downHandler_.reset();
	}
}
