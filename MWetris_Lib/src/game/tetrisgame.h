#ifndef MWETRIS_GAME_TETRISGAME_H
#define MWETRIS_GAME_TETRISGAME_H

#include "device.h"
#include "tetrisparameters.h"
#include "tetrisgameevent.h"
#include "localplayerboard.h"
#include "computer.h"
#include "devicemanager.h"
#include "timerhandler.h"
#include "aiplayer.h"
#include "humanplayer.h"

#include <ai.h>

#include <vector>
#include <memory>

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
		std::string uuid;
	};

	class TetrisGame {
	public:
		mw::PublicSignal<TetrisGame, InitGameEvent> initGameEvent;
		mw::PublicSignal<TetrisGame, GameOver> gameOverEvent;
		mw::PublicSignal<TetrisGame, GamePause> gamePauseEvent;

		TetrisGame(std::shared_ptr<DeviceManager> deviceManager);
		~TetrisGame();

		// Updates everything. Should be called each frame.
		void update(double deltaTime);

		void createDefaultGame(DevicePtr device);

		void createGame(int width, int height, const std::vector<Human>& devices, const std::vector<Ai>& ais);

		bool isPaused() const;

		void setPause(bool pause);

		// Pause/Unpause the game depending on the current state of the game.
		void pause();

		// Restart the active game.
		void restartGame();

		int getNbrOfPlayers() const;

		void saveDefaultGame();

		void setFixTimestep(double delta) {
			fixedTimestep = delta;
		}

		bool isDefaultGame() const;

	private:
		void initGame();
		void updateGame(double deltaTime);
		void applyRules(tetris::BoardEvent gameEvent, int value, const LocalPlayerBoardPtr& playerBoard);

		std::vector<PlayerPtr> players_;

		double accumulator_ = 0.0;
		bool pause_ = false;
		double fixedTimestep = 1.0 / 60.0;

		mw::signals::ScopedConnections connections_;
		TimeHandler timeHandler_;
		TimeHandler::Key pauseKey_;
		std::shared_ptr<DeviceManager> deviceManager_;
	};

}

#endif
