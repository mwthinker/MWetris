#ifndef MWETRIS_GAME_TETRISGAME_H
#define MWETRIS_GAME_TETRISGAME_H

#include "device.h"
#include "tetrisparameters.h"
#include "tetrisgameevent.h"
#include "localplayerboard.h"
#include "computer.h"
#include "timerhandler.h"
#include "aiplayer.h"
#include "humanplayer.h"
#include "remoteplayer.h"
#include "defaultgamerules.h"
#include "playerslot.h"

#include <ai.h>

#include <vector>
#include <memory>

namespace mwetris::game {

	constexpr double CountDownSeconds = 3.0;

	class PlayerFactory {
	public:
		PlayerFactory();

		std::vector<PlayerPtr> createPlayers(int width, int height,
			const std::vector<Human>& humans,
			const std::vector<Ai>& ais
		);

		PlayerPtr createPlayer(int width, int height, const Human& human);

		PlayerPtr createPlayer(int width, int height, const Ai& ai);
	};

	class TetrisGame {
	public:
		mw::PublicSignal<TetrisGame, InitGameEvent> initGameEvent;
		mw::PublicSignal<TetrisGame, GameOver> gameOverEvent;
		mw::PublicSignal<TetrisGame, GamePause> gamePauseEvent;
		mw::PublicSignal<TetrisGame, GameRestart> gameRestartEvent;

		TetrisGame();
		~TetrisGame();

		// Updates everything. Should be called each frame.
		void update(double deltaTime);

		void createDefaultGame(DevicePtr device);

		void createGame(std::unique_ptr<GameRules> gameRules, const std::vector<PlayerPtr>& players);

		bool isPaused() const;

		void setPause(bool pause);

		// Pause/Unpause the game depending on the current state of the game.
		void pause();

		// Restart the active game.
		void restartGame();

		// Restart the active game.
		void restartGame(tetris::BlockType current, tetris::BlockType next);

		int getNbrOfPlayers() const;

		void saveDefaultGame();

		void setFixTimestep(double delta) {
			fixedTimestep = delta;
		}

		bool isDefaultGame() const;

	private:
		void initGame();
		void updateGame(double deltaTime);

		std::vector<PlayerPtr> players_;

		double accumulator_ = 0.0;
		bool pause_ = false;
		double fixedTimestep = 1.0 / 60.0;

		mw::signals::ScopedConnections connections_;
		TimeHandler timeHandler_;
		TimeHandler::Key pauseKey_;
		std::unique_ptr<GameRules> rules_;
	};

}

#endif
