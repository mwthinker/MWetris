#ifndef MWETRIS_GAME_TETRISGAME_H
#define MWETRIS_GAME_TETRISGAME_H

#include "device.h"
#include "playerdata.h"
#include "localgame.h"
#include "tetrisparameters.h"
#include "tetrisgameevent.h"
#include "player.h"
#include "gamerules.h"
#include "remoteplayer.h"

#include <mw/signal.h>

//#include <net/packet.h>
//#include <net/network.h>
//#include <net/connection.h>

#include <vector>
#include <memory>

namespace tetris::game {

	// Handle all communication between game and gui.
	class TetrisGame {
	public:
		enum class Status {WAITING_TO_CONNECT, LOCAL, SERVER, CLIENT};

		TetrisGame();
		~TetrisGame();

		// Updates everything. Should be called each frame.
		void update(double deltaTime);

		// Uses the same settings as last call.
		void createGame(const std::vector<DevicePtr>& devices);

		void createLocalGame(int columns, int rows, const std::vector<DevicePtr>& devices);

		void createServerGame(int port, int columns, int rows, const std::vector<DevicePtr>& devices);

		void createClientGame(int port, std::string ip);

		void resumeGame(int columns, int rows, const std::vector<PlayerData>& playersData);

		void closeGame();

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

		Status getStatus() const {
			return status_;
		}

		mw::signals::Connection addGameEventHandler(const mw::Signal<TetrisGameEvent&>::Callback& callback) {
			return eventHandler_.connect(callback);
		}

		std::vector<PlayerData> getPlayerData() const;

		bool currentGameHasCountDown() const {
			return players_.size() > 1 && COUNT_DOWN_TIME > 0;
		}

		bool isCurrentGameActive() const;

		bool isDefaultGame() const {
			return true; // status_ == TetrisGame::LOCAL && width_ == TETRIS_WIDTH && height_ == TETRIS_HEIGHT && localConnection_.getNbrOfPlayers() == 1;
		}

		bool isCustomGame() const {
			return status_ == Status::LOCAL && !(width_ == TETRIS_WIDTH && height_ == TETRIS_HEIGHT);
		}

	private:
		void createLocalPlayers(int columns, int rows, const std::vector<DevicePtr>& devices);
		void receiveRemotePlayers(const std::vector<std::shared_ptr<RemotePlayer>>& players);

		void initGame();
		void updateGame(double deltaTime);

		void startNewCountDown();
		void updateCurrentCountDown(double deltaTime);
		bool hasActiveCountDown() const;

		void triggerGameStartEvent();
		void triggerTriggerInitGameEvent();

		std::vector<LocalPlayerPtr> players_;

		static constexpr double FIXED_TIMESTEP = 1.0 / 60.0;

		mw::Signal<TetrisGameEvent&> eventHandler_;
		std::unique_ptr<GameManager> game_;

		Status status_{Status::WAITING_TO_CONNECT};
		int width_{TETRIS_WIDTH};
		int height_{TETRIS_HEIGHT};
		int maxLevel_{TETRIS_MAX_LEVEL};

		double timeLeftToStart_{-0.0}; // Time left for the count down.
		int wholeTimeLeft_{0}; // Time left in whole seconds. E.g. timeLeftToStart_ = 1.4s means that wholeTimeLeft_ = 2s;
	};

}

#endif
