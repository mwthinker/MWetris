#ifndef MWETRIS_GAME_TETRISGAME_H
#define MWETRIS_GAME_TETRISGAME_H

#include "device.h"
#include "localgame.h"
#include "tetrisparameters.h"
#include "tetrisgameevent.h"
#include "player.h"
#include "gamerules.h"

#include <vector>
#include <memory>

namespace mwetris::game {
	
	class TetrisGame {
	public:
		mw::PublicSignal<TetrisGame, InitGameEvent> initGameEvent;

		TetrisGame();
		~TetrisGame();

		// Updates everything. Should be called each frame.
		void update(double deltaTime);

		// Uses the same settings as last call.
		void createGame(const std::vector<DevicePtr>& devices);

		void createGame(int columns, int rows, const std::vector<DevicePtr>& devices);

		bool isPaused() const;

		// Pause/Unpause the game depending on the current state of
		// the game.
		void pause();

		// Restart the active game. If the game was not started, nothing happens.
		void restartGame();

		int getNbrOfPlayers() const;

		int getMaxLevel() const {
			return maxLevel_;
		}

		void resizeBoard(int width, int height);

		int getRows() const {
			return height_;
		}

		int getColumns() const {
			return width_;
		}

		bool currentGameHasCountDown() const {
			return players_.size() > 1 && CountDownTime > 0;
		}

		bool isCurrentGameActive() const;

	private:
		void createLocalPlayers(int columns, int rows, const std::vector<DevicePtr>& devices);

		void initGame();
		void updateGame(double deltaTime);

		void startNewCountDown();
		void updateCurrentCountDown(double deltaTime);
		bool hasActiveCountDown() const;

		std::vector<LocalPlayerPtr> players_;

		std::unique_ptr<GameManager> game_;

		int width_{TetrisWidth};
		int height_{TetrisHeight};
		int maxLevel_{TetrisMaxLevel};

		double timeLeftToStart_{-0.0}; // Time left for the count down.
		int wholeTimeLeft_{0}; // Time left in whole seconds. E.g. timeLeftToStart_ = 1.4s means that wholeTimeLeft_ = 2s;
	};

}

#endif
