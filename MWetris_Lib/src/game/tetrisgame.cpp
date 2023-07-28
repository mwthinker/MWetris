#include "tetrisgame.h"

#include "tetrisboard.h"
#include "helper.h"
#include "tetrisgameevent.h"
#include "localplayer.h"
#include "tetrisparameters.h"
#include "localplayerbuilder.h"
#include "serialize.h"

#include <vector>
#include <algorithm>

namespace mwetris::game {

	namespace {

		constexpr int LevelUpNbr = 10;

		std::vector<LocalPlayerPtr> createLocalPlayers(int columns, int rows,
			const std::vector<DevicePtr>& devices,
			const std::vector<tetris::Ai>& ais = {}) {

			std::vector<LocalPlayerPtr> players;

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
				builder.withHeight(rows);
				builder.withWidth(columns);

				auto player = builder.build();
				players.push_back(player);
			}

			return players;
		}

	}

	TetrisGame::TetrisGame() {
	}

	TetrisGame::~TetrisGame() {
	}

	void TetrisGame::saveCurrentGame() {
		int nbrGameOver = 0;
		for (const auto& player : players_) {
			if (player->isGameOver()) {
				++nbrGameOver;
			}
		}
		
		// Only save one player default game.
		if (players_.size() > 1 && nbrGameOver == players_.size() - 1 || players_.size() == 0 || players_.size() == 1 && nbrGameOver == 1) {
			game::clearSavedGame();
		} else {
			saveGame(players_);
		}
	}

	void TetrisGame::resumeGame(const DeviceManager& deviceManager) {
		auto players = loadGame(deviceManager);
		if (players.empty()) {
			createGame(TetrisWidth, TetrisHeight, {deviceManager.getDefaultDevice1()});
		} else {
			players_ = players;

			pause();
			connections_.clear();

			for (auto& player : players_) {
				connections_ += player->gameboardEventUpdate.connect([this, p = player](tetris::BoardEvent gameEvent, int value) {
					applyRules(gameEvent, value, p);
				});
			}

			initGame();
		}
	}


	void TetrisGame::createGame(int columns, int rows, const std::vector<DevicePtr>& devices, const std::vector<tetris::Ai>& ais) {
		connections_.clear();
		players_ = createLocalPlayers(columns, rows, devices);

		for (auto& player : players_) {
			connections_ += player->gameboardEventUpdate.connect([this, p = player](tetris::BoardEvent gameEvent, int value) {
				applyRules(gameEvent, value, p);
			});
		}

		initGame();
	}

	void TetrisGame::createDefaultGame(const DeviceManager& deviceManager) {
		createGame(TetrisWidth, TetrisHeight, {deviceManager.getDefaultDevice1()});
	}

	void TetrisGame::initGame() {
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
		
		connections_.clear();
		for (auto& player : players_) {
			connections_ += player->gameboardEventUpdate.connect([this, p = player](tetris::BoardEvent gameEvent, int value) {
				applyRules(gameEvent, value, p);
			});
			player->updateRestart();
		}

		initGame();
	}

	bool TetrisGame::isPaused() const {
		return pause_;
	}

	void TetrisGame::pause() {
		pause_ = !pause_;
		gamePauseEvent(GamePause{pause_});
		if (pause_) {
			saveGame(players_);
		} else {
			counter = 4;
			countDown = 3;
		}
	}

	int TetrisGame::getNbrOfPlayers() const {
		return static_cast<int>(players_.size());
	}

	void TetrisGame::update(double deltaTime) {
		int tmp = std::ceil(countDown);
		if (countDown >= 0) {
			countDown -= deltaTime;
		}

		if (tmp < counter && counter >= 0) {
			counter = tmp;
			countDownGameEvent(tmp);
		}

		if (!pause_ && counter <= 0) {
			updateGame(deltaTime);
		}
	}

	void TetrisGame::updateGame(double deltaTime) {
		if (deltaTime > 0.250) {
			// To avoid spiral of death.
			deltaTime = 0.250;
		}

		accumulator_ += deltaTime;
		while (accumulator_ >= fixedTimestep) {
			accumulator_ -= fixedTimestep;
			for (auto& player : players_) {
				player->update(fixedTimestep);
			}
		}
	}

	void TetrisGame::applyRules(tetris::BoardEvent gameEvent, int value, const LocalPlayerPtr& player) {
		if (tetris::BoardEvent::RowsRemoved == gameEvent) {
			//rows_ += value;
			//level_ = rows_ / LevelUpNbr + 1;
			//points_ += level_ * value * value;

			//player->updateLevel(level_);
			//player->updatePoints(points_);
		}
	}

}
