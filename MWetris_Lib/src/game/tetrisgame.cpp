#include "tetrisgame.h"

#include "tetrisboard.h"
#include "helper.h"
#include "tetrisgameevent.h"
#include "tetrisparameters.h"
#include "serialize.h"
#include "defaultgamerules.h"
#include "player.h"

#include <vector>
#include <algorithm>

namespace mwetris::game {

	TetrisGame::TetrisGame() {
	}

	TetrisGame::~TetrisGame() {
	}

	bool TetrisGame::isDefaultGame() const {
		if (players_.size() != 1) {
			return false;
		}

		const auto& player = *players_.front();

		if (player.isLocal()) {
			return player.isLocal();
		}

		if (player.getRows() != TetrisHeight) {
			return false;
		}

		if (player.getColumns() != TetrisWidth) {
			return false;
		}

		if (!dynamic_cast<DefaultGameRules*>(rules_.get())) {
			return false;
		}

		return player.isHuman();
	}

	void TetrisGame::saveDefaultGame() {
		if (!isDefaultGame()) {
			return;
		}

		const auto& player = *players_.front();
		if (!player.isGameOver()) {
			saveGame(player);
		}
	}

	void TetrisGame::createDefaultGame(DevicePtr device) {
		rules_ = std::make_unique<DefaultGameRules>();

		if (isDefaultGame() && players_.front()->isGameOver()) {
			rules_->createGame(players_);
			restartGame();
		} else {
			PlayerPtr player = loadGame(device);
			if (!player) {
				DefaultPlayerData playerData{
					.level = 1,
					.points = 0
				};
				tetris::TetrisBoard tetrisBoard{TetrisWidth, TetrisHeight, tetris::randomBlockType(), tetris::randomBlockType()};
				player = createHumanPlayer(device, playerData, std::move(tetrisBoard));
			}
			connections_.clear();
			players_ = {player};

			connections_ += player->playerBoardUpdate.connect([](const PlayerBoardEvent& playerBoardEvent) {
				game::clearSavedGame();
			});

			rules_->createGame(players_);
			setPause(true);
			initGame();
		}
	}

	void TetrisGame::createGame(std::unique_ptr<GameRules> gameRules, const std::vector<PlayerPtr>& players) {
		rules_ = std::move(gameRules);

		players_ = players;
		rules_->createGame(players_);

		initGame();
	}

	void TetrisGame::initGame() {
		initGameEvent.invoke(InitGameEvent{players_.begin(), players_.end()});
	}

	void TetrisGame::restartGame() {
		restartGame(tetris::randomBlockType(), tetris::randomBlockType());
	}

	void TetrisGame::restartGame(tetris::BlockType current, tetris::BlockType next) {
		gameRestartEvent(game::GameRestart{
			.current = current,
			.next = next
		});
		accumulator_ = 0.0;
		setPause(false);

		for (auto& player : players_) {
			player->updateRestart(current, next);
		}
		initGame();
		rules_->restart();
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
		if (isDefaultGame() && players_.size() == 1 && !players_.front()->isGameOver()) {
			saveGame(*players_.front());
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
		rules_->update(deltaTime);
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
