#ifndef MWETRIS_GAME_REMOTEPLAYERBOARD_H
#define MWETRIS_GAME_REMOTEPLAYERBOARD_H

#include "playerboard.h"

namespace mwetris::game {

	class RemotePlayerBoard;
	using RemotePlayerBoardPtr = std::shared_ptr<RemotePlayerBoard>;

	class RemotePlayerBoard : public PlayerBoard {
	public:
		RemotePlayerBoard(const tetris::TetrisBoard& board, const std::string& name);

		RemotePlayerBoard(const RemotePlayerBoard&) = delete;
		RemotePlayerBoard& operator=(const RemotePlayerBoard&) = delete;
		RemotePlayerBoard(RemotePlayerBoard&&) = delete;
		RemotePlayerBoard& operator=(RemotePlayerBoard&&) = delete;

		int getLevel() const override {
			return level_;
		}

		int getPoints() const override {
			return points_;
		}

		int getClearedRows() const override {
			return clearedRows_;
		}

		int getGameOverPosition() const override {
			return gameOverPosition_;
		}

	private:
		void handleBoardEvent(tetris::BoardEvent boardEvent, int value) override;

		int points_ = 0;
		int level_ = 1;
		int clearedRows_ = 0;
		int gameOverPosition_ = 0;
	};

}

#endif
