#ifndef TETRISGAME_H
#define TETRISGAME_H

#include "protocol.h"
#include "device.h"
#include "playerdata.h"
#include "localgame.h"
#include "tetrisparameters.h"

#include <mw/signal.h>

//#include <net/packet.h>
//#include <net/network.h>
//#include <net/connection.h>

#include <vector>
#include <memory>

namespace tetris {

	class TetrisGameEvent;
	class Player;
	class GameRules;
	class IGameManager;
	class RemotePlayer;

	// Handle all communication between game and gui.
	class TetrisGame {
	public:
		enum Status { WAITING_TO_CONNECT, LOCAL, SERVER, CLIENT };

		TetrisGame();
		~TetrisGame();

		// Updates everything. Should be called each frame.
		void update(double deltaTime);

		// Uses the same settings as last call.
		void createGame(const std::vector<IDevicePtr>& devices);

		void createLocalGame(int columns, int rows, const std::vector<IDevicePtr>& devices);

		void createServerGame(int port, int columns, int rows, const std::vector<IDevicePtr>& devices);

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
			return status_ == TetrisGame::LOCAL && !(width_ == TETRIS_WIDTH && height_ == TETRIS_HEIGHT);
		}

	private:
		void createLocalPlayers(int columns, int rows, const std::vector<IDevicePtr>& devices);
		void receiveRemotePlayers(const std::vector<std::shared_ptr<RemotePlayer>>& players);

		void initGame();
		void updateGame(double deltaTime);

		void startNewCountDown();
		void updateCurrentCountDown(double deltaTime);
		bool hasActiveCountDown() const;

		void triggerGameStartEvent();
		void triggerTriggerInitGameEvent();

		std::vector<ILocalPlayerPtr> players_;

		static const double FIXED_TIMESTEP;

		mw::Signal<TetrisGameEvent&> eventHandler_;
		std::unique_ptr<IGameManager> game_;

		Status status_;
		int width_, height_, maxLevel_;

		double timeLeftToStart_; // Time left for the count down.
		int wholeTimeLeft_; // Time left in whole seconds. E.g. timeLeftToStart_ = 1.4s means that wholeTimeLeft_ = 2s;
	};

}

#endif // TETRISGAME_H
