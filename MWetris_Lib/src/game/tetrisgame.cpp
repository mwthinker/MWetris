#include "tetrisgame.h"

#include "tetrisboard.h"
#include "helper.h"
#include "tetrisgameevent.h"
#include "localplayerboard.h"
#include "tetrisparameters.h"
#include "localplayerboardbuilder.h"
#include "serialize.h"

#include <vector>
#include <algorithm>

namespace mwetris::game {

	namespace {

		constexpr int LevelUpNbr = 10;

		LocalPlayerBoardPtr createLocalPlayerBoard(int width, int height, const std::string& name) {
			return LocalPlayerBoardBuilder{}
				.withClearedRows(0)
				.withGameOverPosition(0)
				.withLevel(1)
				.withLevelUpCounter(0)
				.withName(name)
				.withMovingBlockType(tetris::randomBlockType())
				.withNextBlockType(tetris::randomBlockType())
				.withPoints(0)
				.withWidth(width)
				.withHeight(height)
				.build();
		}

	}

	TetrisGame::TetrisGame(std::shared_ptr<DeviceManager> deviceManager)
		: deviceManager_{deviceManager} {
	}

	TetrisGame::~TetrisGame() {
		game::loadGame();
	}

	bool TetrisGame::isDefaultGame() const {
		if (players_.size() != 1) {
			return false;
		}

		const auto& player = *players_.front();

		if (player.getPlayerBoard()->getTetrisBoard().getRows() != TetrisHeight) {
			return false;
		}

		if (player.getPlayerBoard()->getTetrisBoard().getColumns() != TetrisWidth) {
			return false;
		}

		return dynamic_cast<const HumanPlayer*>(&player);
	}

	void TetrisGame::saveDefaultGame() {
		if (!isDefaultGame()) {
			return;
		}

		const auto& player = static_cast<HumanPlayer&>(*players_.front());
		if (!player.getPlayerBoard()->isGameOver()) {
			saveGame(*player.getPlayerBoard());
		}
	}

	void TetrisGame::createDefaultGame(DevicePtr device) {
		saveDefaultGame();

		if (isDefaultGame() && players_.size() == 1 && players_.front()->getPlayerBoard()->isGameOver()) {
			restartGame();
		} else {
			PlayerPtr player;
			if (auto board = loadGame(); board) {
				player = std::make_shared<HumanPlayer>(device, board);
			} else {
				player = std::make_shared<HumanPlayer>(device, createLocalPlayerBoard(TetrisWidth, TetrisHeight, "Player"));
			}
			connections_.clear();
			players_ = {player};
			connections_ += player->addEventCallback([player](tetris::BoardEvent gameEvent, int value) {
				game::clearSavedGame();
			});
			setPause(true);
			initGame();
		}
	}

	void TetrisGame::createGame(int width, int height, const std::vector<Human>& humans, const std::vector<Ai>& ais) {
		saveDefaultGame();

		for (auto& human : humans) {
			auto board = createLocalPlayerBoard(width, height, human.name);
			auto player = std::make_shared<HumanPlayer>(human.device, createLocalPlayerBoard(TetrisWidth, TetrisHeight, "Player"));
			players_.push_back(player);
		}
		for (auto& ai : ais) {
			auto localPlayer = createLocalPlayerBoard(width, height, ai.name);
			auto player = std::make_shared<AiPlayer>(ai.ai, createLocalPlayerBoard(TetrisWidth, TetrisHeight, "Player"));
			players_.push_back(player);
		}

		/*
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
		*/

		initGame();
	}

	void TetrisGame::initGame() {
		std::vector<PlayerBoardPtr> playerBoards;
		for (auto& player : players_) {
			playerBoards.push_back(player->getPlayerBoard());
		}
		initGameEvent.invoke(InitGameEvent{playerBoards.begin(), playerBoards.end()});

		for (auto& player : playerBoards) {
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

		setPause(false);

		for (auto& player : players_) {
			player->updateRestart();
		}
		initGame();
	}

	bool TetrisGame::isPaused() const {
		return pause_;
	}

	void TetrisGame::setPause(bool pause) {
		timeHandler_.removeCallback(pauseKey_);
		pause_ = pause;
		gamePauseEvent(GamePause{
			.countDown = 0,
			.pause = pause
		});
	}

	void TetrisGame::pause() {
		if (isDefaultGame() && players_.size() == 1 && !players_.front()->getPlayerBoard()->getTetrisBoard().isGameOver()) {
			saveGame(*players_.front()->getPlayerBoard());
		}
		if (timeHandler_.removeCallback(pauseKey_) || !pause_) {
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
			for (auto& player : players_) {
				player->update(fixedTimestep);
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

}
