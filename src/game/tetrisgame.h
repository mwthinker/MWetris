#ifndef MWETRIS_GAME_TETRISGAME_H
#define MWETRIS_GAME_TETRISGAME_H

#include "device.h"
#include "localgame.h"
#include "tetrisparameters.h"
#include "tetrisgameevent.h"
#include "player.h"

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

		// Pause/Unpause the game depending on the current state of the game.
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

		void saveCurrentGame();

	private:
		void createLocalPlayers(int columns, int rows, const std::vector<DevicePtr>& devices);

		void initGame();
		void updateGame(double deltaTime);

		std::vector<LocalPlayerPtr> players_;
		LocalGame localGame_;

		int width_ = TetrisWidth;
		int height_ = TetrisHeight;
		int maxLevel_ = TetrisMaxLevel;

		double accumulator_ = 0.0;
	};

}

#endif
