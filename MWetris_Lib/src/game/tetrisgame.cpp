#include "tetrisgame.h"

#include "tetrisboard.h"
#include "helper.h"
#include "tetrisgameevent.h"
#include "localplayer.h"
#include "tetrisparameters.h"
#include "localplayerbuilder.h"
#include "localgame.h"
#include "serialize.h"

#include <vector>
#include <algorithm>

namespace mwetris::game {

	namespace {

		constexpr double FixedTimestep = 1.0 / 60.0;

	}

	TetrisGame::TetrisGame() {
	}

	TetrisGame::~TetrisGame() {
	}

	void TetrisGame::createLocalPlayers(int columns, int rows, const std::vector<DevicePtr>& devices) {
		players_.clear();
		LocalPlayerBuilder builder;
		for (const auto& device : devices) {
			builder.withDevice(device);
			builder.withClearedRows(0);
			builder.withGameOverPosition(0);
			builder.withLevel(1);
			builder.withLevelUpCounter(0);
			//builder.widthMovingBlock(data.current_);
			//builder.widthName(data.name_);
			builder.withMovingBlockType(tetris::randomBlockType());
			builder.withNextBlockType(tetris::randomBlockType());
			builder.withPoints(0);
			builder.withHeight(height_);
			builder.withWidth(width_);

			auto player = builder.build();
			players_.push_back(player);
		}
	}

	void TetrisGame::saveCurrentGame() {
		int nbrGameOver = 0;
		for (const auto& player : players_) {
			if (player->isGameOver()) {
				++nbrGameOver;
			}
		}
		
		if (players_.size() > 1 && nbrGameOver == players_.size() - 1 || players_.size() == 0 || players_.size() == 1 && nbrGameOver == 1) {
			game::clearSavedGame();
		} else {
			saveGame(players_);
		}
	}

	void TetrisGame::resumeGame(const std::vector<DevicePtr>& devices) {
		auto players = loadGame(devices);
		if (!players.empty() && players.size() <= devices.size()) {
			players_ = players;

			if (players_.size() == 1) {
				localGame_.createGame(players_.front());
			}

			initGame();
		} else {
			createGame(width_, height_, devices);
		}
	}

	void TetrisGame::createGame(int columns, int rows, const std::vector<DevicePtr>& devices) {
		width_ = columns;
		height_ = rows;

		createLocalPlayers(columns, rows, devices);
		if (players_.size() == 1) {
			localGame_.createGame(players_.front());
		}

		initGame();
	}

	void TetrisGame::createGame(const std::vector<DevicePtr>& devices) {
		createGame(width_, height_, devices);
	}

	void TetrisGame::initGame() {
		connections_.clear();
		initGameEvent.invoke(InitGameEvent{players_.begin(), players_.end()});

		for (const auto& player : players_) {
			connections_ += player->gameboardEventUpdate.connect([this, player = player](tetris::BoardEvent event, int) {
				if (event == tetris::BoardEvent::GameOver) {
					gameOverEvent(GameOver{player});
				}
			});
		}
	}

	void TetrisGame::restartGame() {
		accumulator_ = 0.0;
		localGame_.restartGame();

		initGame();
	}

	bool TetrisGame::isPaused() const {
		return pause_;
	}

	void TetrisGame::pause() {
		pause_ = !pause_;
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

			localGame_.restartGame();

			initGame();
		}
	}

	void TetrisGame::update(double deltaTime) {
		if (pause_) {
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
