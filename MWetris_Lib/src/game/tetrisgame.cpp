#include "tetrisgame.h"

#include "tetrisboard.h"
#include "helper.h"
#include "tetrisgameevent.h"
#include "localplayer.h"
#include "tetrisparameters.h"
#include "localplayerbuilder.h"
#include "serialize.h"
#include "snapshot.h"

#include <vector>
#include <algorithm>

namespace mwetris::game {

	namespace {

		constexpr int LevelUpNbr = 10;

		LocalPlayerPtr createLocalPlayer(int columns, int rows, const std::string& name) {
			return LocalPlayerBuilder{}
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
		auto players = loadGame(deviceManager);
		if (players.empty()) {
			createDefaultGame(deviceManager);
		} else {
			playerDevices_ = players;
			players_.clear();
			computers_.clear();
			for (auto& [device, player] : players) {
				players_.push_back(player);
			}

			pause();
			connections_.clear();

			for (auto& [device, player] : playerDevices_) {
				connections_ += player->gameboardEventUpdate.connect([this, p = player](tetris::BoardEvent gameEvent, int value) {
					applyRules(gameEvent, value, p);
				});
			}

			initGame();
		}
	}

	void TetrisGame::createGame(int columns, int rows, const std::vector<Human>& humans, const std::vector<Ai>& ais) {
		connections_.clear();
		players_.clear();
		playerDevices_.clear();

		for (auto& human : humans) {
			auto localPlayer = createLocalPlayer(columns, rows, human.name);
			players_.push_back(localPlayer);
			playerDevices_.emplace_back(human.device, localPlayer);
		}
		for (auto& ai : ais) {
			auto localPlayer = createLocalPlayer(columns, rows, ai.name);
			players_.push_back(localPlayer);
			computers_[std::make_shared<Computer>(ai.ai)] = localPlayer;
		}

		for (auto& [device, player] : playerDevices_) {
			connections_ += player->gameboardEventUpdate.connect([this, p = player](tetris::BoardEvent gameEvent, int value) {
				applyRules(gameEvent, value, p);
			});
		}
		for (auto& [computer, player] : computers_) {
			connections_ += player->gameboardEventUpdate.connect([this, computer = computer, player = player](tetris::BoardEvent boardEvent, int value) {
				computer->onGameboardEvent(player->getTetrisBoard(), boardEvent, value);
				applyRules(boardEvent, value, player);
			});
		}

		initGame();
	}

	void TetrisGame::createDefaultGame(const DeviceManager& deviceManager) {
		createGame(TetrisWidth, TetrisHeight, {
			Human{.name = "Player", .device = deviceManager.getDefaultDevice1()}
		});
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
		
		for (auto& [device, player] : playerDevices_) {
			/*
			connections_ += player->gameboardEventUpdate.connect([this, player = player](tetris::BoardEvent gameEvent, int value) {
				applyRules(gameEvent, value, p);
			});
			*/
			player->updateRestart();
		}
		for (auto& [computer, player] : computers_) {
			/*
			connections_ += player->gameboardEventUpdate.connect([this, player = player](tetris::BoardEvent gameEvent, int value) {
				applyRules(gameEvent, value, p);
			});
			*/
			player->updateRestart();
		}

		initGame();
	}

	bool TetrisGame::isPaused() const {
		return pause_;
	}

	void TetrisGame::pause() {
		if (!pause_) {
			pause_ = true;
			saveGame(playerDevices_);
			gamePauseEvent(GamePause{pause_});
		} else {
			countDownGameEvent(CountDown{3});
			timeHandler_.scheduleRepeat([&, nbr = 2]() mutable {
				if (nbr == 0) {
					pause_ = false;
					countDownGameEvent(CountDown{0});
					gamePauseEvent(GamePause{pause_});
				} else {
					countDownGameEvent(CountDown{nbr--});
				}
			}, 1.0, 3);
		}
	}

	int TetrisGame::getNbrOfPlayers() const {
		return static_cast<int>(players_.size());
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
