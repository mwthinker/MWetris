#include "tetrisgame.h"

#include "tetrisboard.h"
#include "helper.h"
#include "tetrisgameevent.h"
#include "localplayerboard.h"
#include "tetrisparameters.h"
#include "localplayerboardbuilder.h"
#include "serialize.h"
#include "defaultgamerules.h"

#include <vector>
#include <algorithm>

namespace mwetris::game {

	namespace {

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
		remotePlayers_.clear();
		saveDefaultGame();

		if (isDefaultGame() && players_.size() == 1 && players_.front()->getPlayerBoard()->isGameOver()) {
			restartGame();
		} else {
			LocalPlayerBoardPtr localPlayerBoard = loadGame();
			if (!localPlayerBoard) {
				localPlayerBoard = createLocalPlayerBoard(TetrisWidth, TetrisHeight, "Player");
			}
			auto player = std::make_shared<HumanPlayer>(device, localPlayerBoard);
			connections_.clear();
			players_ = {player};
			connections_ += player->addEventCallback([](tetris::BoardEvent gameEvent, int value) {
				game::clearSavedGame();
			});
			setPause(true);
			initGame();
		}
	}

	void TetrisGame::createGame(int width, int height,
		const std::vector<Human>& humans,
		const std::vector<Ai>& ais,
		const std::vector<RemotePlayerPtr>& remotePlayers) {

		saveDefaultGame();
		players_.clear();
		remotePlayers_.clear();

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
		remotePlayers_ = remotePlayers;

		initGame();
	}

	void TetrisGame::initGame() {
		std::vector<PlayerBoardPtr> playerBoards;
		for (auto& player : players_) {
			playerBoards.push_back(player->getPlayerBoard());
		}
		for (auto& player : remotePlayers_) {
			playerBoards.push_back(player->getPlayerBoard());
		}

		initGameEvent.invoke(InitGameEvent{playerBoards.begin(), playerBoards.end()});
	}

	void TetrisGame::restartGame() {
		accumulator_ = 0.0;
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

}
