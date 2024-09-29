#include "tetrisgame.h"

#include "tetrisgameevent.h"
#include "tetrisparameters.h"
#include "serialize.h"
#include "defaultgamerules.h"
#include "player.h"

#include <tetris/helper.h>
#include <tetris/tetrisboard.h>

#include <vector>
#include <algorithm>

namespace app::game {

	TetrisGame::TetrisGame() {
	}

	TetrisGame::~TetrisGame() {
	}

	void TetrisGame::createGame(const std::vector<PlayerPtr>& players) {
		countDown_ = players.size() > 1; // Only countdown if multiplayer.
		players_ = players;
		timeHandler_.reset();
		paused_ = true;
		setPause(false);
	}

	void TetrisGame::restart() {
		timeHandler_.reset();
		paused_ = true;
		setPause(false);
	}

	bool TetrisGame::isPaused() const {
		return paused_;
	}

	void TetrisGame::setPause(bool pauseGame) {
		if (paused_ == pauseGame) {
			// No change, do nothing.
			return;
		}

		if (pauseGame) {
			pause();
		} else if (!timeHandler_.hasKey(pauseKey_)) {
			// Unpause the game only if unpausing is not already scheduled.
			unPause();
		}
	}

	void TetrisGame::unPause() {
		if (countDown_) {
			paused_ = true;
			gamePauseEvent(GamePause{
				.countDown = 3,
				.pause = true
			});
			pauseKey_ = timeHandler_.scheduleRepeat([&, nbr = 2]() mutable {
				if (nbr == 0) {
					paused_ = false;
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
		} else {
			paused_ = false;
			gamePauseEvent(GamePause{
				.countDown = 0,
				.pause = false
			});
		}
	}

	void TetrisGame::pause() {
		paused_ = true;
		timeHandler_.removeCallback(pauseKey_);
		gamePauseEvent(GamePause{
			.countDown = 0,
			.pause = true
		});
	}

	int TetrisGame::getNbrOfPlayers() const {
		return static_cast<int>(players_.size());
	}

	void TetrisGame::update(double deltaTime) {
		timeHandler_.update(deltaTime);

		if (!isPaused()) {
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
