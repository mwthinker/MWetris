#include "tetrisgame.h"

#include "tetrisboard.h"
#include "helper.h"
#include "tetrisgameevent.h"
#include "localplayer.h"
#include "tetrisparameters.h"
#include "gamerules.h"
#include "localplayerbuilder.h"
#include "localgame.h"

#include <vector>
#include <algorithm>

namespace mwetris::game {

	namespace {

		constexpr double FixedTimestep = 1.0 / 60.0;

	}

	TetrisGame::TetrisGame()
		: game_{std::make_unique<LocalGame>()} {
	}

	TetrisGame::~TetrisGame() {
	}

	void TetrisGame::createLocalPlayers(int columns, int rows, const std::vector<DevicePtr>& devices) {
		players_.clear();
		LocalPlayerBuilder builder;
		for (const auto& device : devices) {
			builder.widthDevice(device);
			builder.widthClearedRows(0);
			builder.widthGameOverPosition(0);
			builder.widthLevel(1);
			builder.widthLevelUpCounter(0);
			//builder.widthMovingBlock(data.current_);
			//builder.widthName(data.name_);
			builder.widthMovingBlockType(tetris::randomBlockType());
			builder.widthNextBlockType(tetris::randomBlockType());
			builder.widthPoints(0);
			builder.withHeight(height_);
			builder.withWidth(width_);

			auto player = builder.build();
			players_.push_back(player);
		}
	}

	void TetrisGame::createGame(int columns, int rows, const std::vector<DevicePtr>& devices) {
		width_ = columns;
		height_ = rows;

		createLocalPlayers(columns, rows, devices);
		game_->createGame(players_);
		initGame();
	}

	void TetrisGame::createGame(const std::vector<DevicePtr>& devices) {
		createGame(width_, height_, devices);
	}

	void TetrisGame::initGame() {
		initGameEvent.invoke(InitGameEvent{players_.begin(), players_.end()});

		if (game_->isPaused()) {
			game_->setPaused(false);
		}
	}

	void TetrisGame::restartGame() {
		accumulator_ = 0.0;
		game_->restartGame();

		initGame();
	}

	bool TetrisGame::isPaused() const {
		return game_->isPaused();
	}

	void TetrisGame::pause() {
		game_->setPaused(!game_->isPaused());
		//eventHandler_(GamePause{game_->isPaused(), true});
	}

	int TetrisGame::getNbrOfPlayers() const {
		return static_cast<int>(players_.size());
	}

	void TetrisGame::resizeBoard(int width, int height) {
		if (width > TetrisMinWidth && width <= TetrisMaxWidth &&
			height > TetrisMinHeight && height <= TetrisMaxHeight &&
			(width_ != width || height_ != height)) {

			width_ = width;
			height_ = height;

			game_->createGame(players_);
			initGame();
		}
	}

	void TetrisGame::update(double deltaTime) {
		if (!game_->isPaused()) {
			updateGame(deltaTime);
		}
	}

	void TetrisGame::updateGame(double deltaTime) {
		if (deltaTime > 0.250) {
			// To avoid spiral of death.
			deltaTime = 0.250;
		}

		accumulator_ += deltaTime;
		
		while (accumulator_ >= FixedTimestep) {
			accumulator_ -= FixedTimestep;
			for (auto& player : players_) {
				player->update(FixedTimestep);
			}
		}
	}

}
