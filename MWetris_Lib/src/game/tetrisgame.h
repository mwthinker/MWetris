#ifndef MWETRIS_GAME_TETRISGAME_H
#define MWETRIS_GAME_TETRISGAME_H

#include "device.h"
#include "localgame.h"
#include "tetrisparameters.h"
#include "tetrisgameevent.h"
#include "player.h"

#include <ai.h>

#include <vector>
#include <memory>

namespace mwetris::game {

	constexpr double CountDownSeconds = 3.0;

	class TetrisGame {
	public:
		mw::PublicSignal<TetrisGame, InitGameEvent> initGameEvent;
		mw::PublicSignal<TetrisGame, CountDown> countDownGameEvent;
		mw::PublicSignal<TetrisGame, GameOver> gameOverEvent;
		mw::PublicSignal<TetrisGame, GamePause> gamePauseEvent;

		TetrisGame();
		~TetrisGame();

		// Updates everything. Should be called each frame.
		void update(double deltaTime);

		void resumeGame(const DeviceManager& deviceManager);

		// Uses the same settings as last call.
		void createGame(const std::vector<DevicePtr>& devices, const std::vector<tetris::Ai>& ais = {});

		void createGame(int columns, int rows, const std::vector<DevicePtr>& devices, const std::vector<tetris::Ai>& ais = {});

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
		void initGame();
		void updateGame(double deltaTime);

		std::vector<LocalPlayerPtr> players_;
		LocalGame localGame_;
		mw::signals::ScopedConnections connections_;

		int width_ = TetrisWidth;
		int height_ = TetrisHeight;
		int maxLevel_ = TetrisMaxLevel;

		double countDown = 0;
		int counter = 0;

		double accumulator_ = 0.0;
		bool pause_ = false;
	};

}

#endif
