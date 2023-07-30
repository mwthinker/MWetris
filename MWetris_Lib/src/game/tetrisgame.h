#ifndef MWETRIS_GAME_TETRISGAME_H
#define MWETRIS_GAME_TETRISGAME_H

#include "device.h"
#include "tetrisparameters.h"
#include "tetrisgameevent.h"
#include "localplayer.h"
#include "computer.h"
#include "devicemanager.h"

#include <ai.h>

#include <vector>
#include <memory>
#include <unordered_map>

namespace mwetris::game {

	constexpr double CountDownSeconds = 3.0;

	struct Human {
		std::string name;
		DevicePtr device;
	};

	struct Ai {
		std::string name;
		tetris::Ai ai;
	};

	struct Remote {

	};

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

		void createDefaultGame(const DeviceManager& deviceManager);

		void createGame(int columns, int rows, const std::vector<Human>& humans, const std::vector<Ai>& ais = {});

		bool isPaused() const;

		// Pause/Unpause the game depending on the current state of the game.
		void pause();

		// Restart the active game.
		void restartGame();

		int getNbrOfPlayers() const;

		void saveCurrentGame();

		void setFixTimestep(double delta) {
			fixedTimestep = delta;
		}

	private:
		void initGame();
		void updateGame(double deltaTime);
		void applyRules(tetris::BoardEvent gameEvent, int value, const LocalPlayerPtr& player);

		std::unordered_map<DevicePtr, LocalPlayerPtr> humans_;
		std::unordered_map<ComputerPtr, LocalPlayerPtr> computers_;
		std::vector<PlayerPtr> players_;

		double countDown = 0;
		int counter = 0;

		double accumulator_ = 0.0;
		bool pause_ = false;
		double fixedTimestep = 1.0 / 60.0;

		mw::signals::ScopedConnections connections_;
	};

}

#endif
