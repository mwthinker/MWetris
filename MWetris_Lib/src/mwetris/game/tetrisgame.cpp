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

namespace mwetris::game {

	TetrisGame::TetrisGame() {
	}

	TetrisGame::~TetrisGame() {
	}

	void TetrisGame::createGame(const std::vector<PlayerPtr>& players) {
		connections_.clear();
		players_ = players;
		
		setPause(true);
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
