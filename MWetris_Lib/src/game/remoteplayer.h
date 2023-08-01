#ifndef MWETRIS_GAME_REMOTEPLAYER_H
#define MWETRIS_GAME_REMOTEPLAYER_H

#include "player.h"

namespace mwetris::game {

	class RemotePlayer;
	using RemotePlayerPtr = std::shared_ptr<RemotePlayer>;

	class RemotePlayer : public Player {
	public:
		RemotePlayer(const tetris::TetrisBoard& board, const std::string& name);

		RemotePlayer(const RemotePlayer&) = delete;
		RemotePlayer& operator=(const RemotePlayer&) = delete;
		RemotePlayer(RemotePlayer&&) = delete;
		RemotePlayer& operator=(RemotePlayer&&) = delete;

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
