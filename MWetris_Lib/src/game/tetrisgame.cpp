#include "tetrisgame.h"

#include "tetrisboard.h"
#include "helper.h"
#include "tetrisgameevent.h"
#include "localplayerboard.h"
#include "tetrisparameters.h"
#include "localplayerboardbuilder.h"
#include "serialize.h"
#include "snapshot.h"

#include <vector>
#include <algorithm>

namespace mwetris::game {

	namespace {

		constexpr int LevelUpNbr = 10;

		LocalPlayerBoardPtr createLocalPlayer(int columns, int rows, const std::string& name) {
			return LocalPlayerBoardBuilder{}
				.withClearedRows(0)
				.withGameOverPosition(0)
				.withLevel(1)
				.withLevelUpCounter(0)
				.withName(name)
				.withMovingBlockType(tetris::randomBlockType())
				.withNextBlockType(tetris::randomBlockType())
				.withPoints(0)
				.withHeight(rows)
				.withWidth(columns)
				.build();
		}

	}

	TetrisGame::TetrisGame() {
	}

	TetrisGame::~TetrisGame() {
	}

	void TetrisGame::saveCurrentGame() {
		int nbrGameOver = 0;
		for (const auto& [device, player] : playerDevices_) {
			if (player->isGameOver()) {
				++nbrGameOver;
			}
		}
		
		// Only save one player default game.
		if (playerDevices_.size() > 1 && nbrGameOver == playerDevices_.size() - 1 || playerDevices_.size() == 0 || playerDevices_.size() == 1 && nbrGameOver == 1) {
			game::clearSavedGame();
		} else {
			saveGame(playerDevices_);
		}
	}

	void TetrisGame::resumeGame(const DeviceManager& deviceManager) {
		auto playerBoards = loadGame(deviceManager);
		if (playerBoards.empty()) {
			createDefaultGame(deviceManager);
		} else {
			playerDevices_ = playerBoards;
			playerBoards_.clear();
			computers_.clear();
			for (auto& [device, playerBoard] : playerBoards) {
				playerBoards_.push_back(playerBoard);
			}

			pause();
			connections_.clear();

			for (auto& [device, playerBoard] : playerDevices_) {
				connections_ += playerBoard->gameboardEventUpdate.connect([this, playerBoard = playerBoard](tetris::BoardEvent gameEvent, int value) {
					applyRules(gameEvent, value, playerBoard);
				});
			}

			initGame();
		}
	}

	void TetrisGame::createGame(int columns, int rows, const std::vector<Human>& humans, const std::vector<Ai>& ais) {
		connections_.clear();
		playerBoards_.clear();
		playerDevices_.clear();

		for (auto& human : humans) {
			auto localPlayer = createLocalPlayer(columns, rows, human.name);
			playerBoards_.push_back(localPlayer);
			playerDevices_.emplace_back(human.device, localPlayer);
		}
		for (auto& ai : ais) {
			auto localPlayer = createLocalPlayer(columns, rows, ai.name);
			playerBoards_.push_back(localPlayer);
			computers_.emplace_back(std::make_shared<Computer>(ai.ai), localPlayer);
		}

		for (auto& [device, playerBoard] : playerDevices_) {
			connections_ += playerBoard->gameboardEventUpdate.connect([this, p = playerBoard](tetris::BoardEvent gameEvent, int value) {
				applyRules(gameEvent, value, p);
			});
		}
		for (auto& [computer, playerBoard] : computers_) {
			connections_ += playerBoard->gameboardEventUpdate.connect([this, computer = computer, playerBoard = playerBoard](tetris::BoardEvent boardEvent, int value) {
				computer->onGameboardEvent(playerBoard->getTetrisBoard(), boardEvent, value);
				applyRules(boardEvent, value, playerBoard);
			});
		}

		initGame();
	}

	void TetrisGame::createDefaultGame(const DeviceManager& deviceManager) {
		if (isDefaultGame()) {
			restartGame();
		} else {
			createGame(TetrisWidth, TetrisHeight, {
				Human{.name = "Player", .device = deviceManager.getDefaultDevice1()}
				});
		}
	}

	void TetrisGame::initGame() {
		initGameEvent.invoke(InitGameEvent{playerBoards_.begin(), playerBoards_.end()});

		for (const auto& player : playerBoards_) {
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

		for (auto& [device, playerBoard] : playerDevices_) {
			/*
			connections_ += player->gameboardEventUpdate.connect([this, player = player](tetris::BoardEvent gameEvent, int value) {
				applyRules(gameEvent, value, p);
			});
			*/
			playerBoard->updateRestart();
		}
		for (auto& [computer, playerBoard] : computers_) {
			/*
			connections_ += player->gameboardEventUpdate.connect([this, player = player](tetris::BoardEvent gameEvent, int value) {
				applyRules(gameEvent, value, p);
			});
			*/
			playerBoard->updateRestart();
		}

		initGame();
	}

	bool TetrisGame::isPaused() const {
		return pause_;
	}

	void TetrisGame::pause() {
		if (timeHandler_.removeCallback(pauseKey_) || !pause_) {
			if (!pause_) {
				saveGame(playerDevices_);
			}
			gamePauseEvent(GamePause{
				.countDown = 0,
				.pause = true
				});
			pause_ = true;
		} else {
			gamePauseEvent(GamePause{
				.countDown = 3,
				.pause = true
				});
			pauseKey_ = timeHandler_.scheduleRepeat([&, nbr = 2]() mutable {
				if (nbr == 0) {
					pause_ = false;
					gamePauseEvent(GamePause{
						.countDown = 0,
						.pause = false
						});
				} else {
					gamePauseEvent(GamePause{
						.countDown = nbr--,
						.pause = true
						});
				}
			}, 1.0, 3);
		}
	}

	int TetrisGame::getNbrOfPlayers() const {
		return static_cast<int>(playerBoards_.size());
	}

	void TetrisGame::update(double deltaTime) {
		timeHandler_.update(deltaTime);

		if (!pause_) {
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
			for (auto& [device, player] : playerDevices_) {
				player->update(device->getInput(), fixedTimestep);
			}
			for (auto& [computer, player] : computers_) {
				player->update(computer->getInput(), fixedTimestep);
			}
		}
	}

	void TetrisGame::applyRules(tetris::BoardEvent gameEvent, int value, const LocalPlayerBoardPtr& playerBoard) {
		if (tetris::BoardEvent::RowsRemoved == gameEvent) {
			//rows_ += value;
			//level_ = rows_ / LevelUpNbr + 1;
			//points_ += level_ * value * value;

			//player->updateLevel(level_);
			//player->updatePoints(points_);
		}
	}

	bool TetrisGame::isDefaultGame() const { // TODO! Remove magic numbers!
		return playerBoards_.size() == 1 && playerDevices_.size() == 1 &&
			playerBoards_.front()->getTetrisBoard().getRows() == 24 &&
			playerBoards_.front()->getTetrisBoard().getColumns() == 10;
	}

}
