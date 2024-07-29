#ifndef MWETRIS_GAME_TETRISGAME_H
#define MWETRIS_GAME_TETRISGAME_H

#include "device.h"
#include "tetrisparameters.h"
#include "tetrisgameevent.h"
#include "computer.h"
#include "../timerhandler.h"
#include "defaultgamerules.h"
#include "playerslot.h"

#include <tetris/ai.h>

#include <vector>
#include <memory>

namespace mwetris::game {

	constexpr double CountDownSeconds = 3.0;

	class TetrisGame {
	public:
		mw::PublicSignal<TetrisGame, GameOver> gameOverEvent;
		mw::PublicSignal<TetrisGame, GamePause> gamePauseEvent;

		TetrisGame();
		~TetrisGame();

		// Updates everything. Should be called each frame.
		void update(double deltaTime);

		void createGame(const std::vector<PlayerPtr>& players);

		void restart();

		bool isPaused() const;

		void setPause(bool pause);

		int getNbrOfPlayers() const;

		void setFixTimestep(double delta) {
			fixedTimestep = delta;
		}

		const std::vector<PlayerPtr>& getPlayers() const {
			return players_;
		}

	private:
		void updateGame(double deltaTime);

		// Pause/Unpause the game depending on the current state of the game.
		void pause();
		void unPause();

		std::vector<PlayerPtr> players_;

		double accumulator_ = 0.0;
		double fixedTimestep = 1.0 / 60.0;

		bool paused_ = false;
		bool countDown_ = false;
		TimeHandler timeHandler_;
		TimeHandler::Key pauseKey_;
	};

}

#endif
