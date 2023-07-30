#ifndef MWETRIS_GAME_LOCALPLAYER_H
#define MWETRIS_GAME_LOCALPLAYER_H

#include "player.h"
#include "actionhandler.h"

namespace mwetris::game {

	class LocalPlayer;
	using LocalPlayerPtr = std::shared_ptr<LocalPlayer>;

	class LocalPlayer : public Player {
	public:
		LocalPlayer(const tetris::TetrisBoard& board, const std::string& name);

		LocalPlayer(const LocalPlayer&) = delete;
		LocalPlayer& operator=(const LocalPlayer&) = delete;
		LocalPlayer(LocalPlayer&&) = delete;
		LocalPlayer& operator=(LocalPlayer&&) = delete;

		void update(Input input, double deltaTime);

		void addRow(int holes);

		void updatePoints(int points);

		void updateName(const std::string& name);

		void updateLevel(int level);

		void updateClearedRows(int clearedRows);

		void updateGameOverPosition(int gameOverPosition);

		void updateRestart();

		void updateGameOver();

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

		float getGravityDownSpeed() const {
			return 1 + level_ * 0.5f;
		}

	private:
		void handleBoardEvent(tetris::BoardEvent boardEvent, int value) override;

		int points_ = 0;
		int level_ = 1;
		int clearedRows_ = 0;
		int gameOverPosition_ = 0;

		ActionHandler leftHandler_{0.09, false};
		ActionHandler rightHandler_{0.09, false};
		ActionHandler rotateHandler_{0.0, true};
		ActionHandler downGroundHandler_{0.0, true};
		ActionHandler gravityMove_{1, false};  // Value doesn't matter! Changes every frame.
		ActionHandler downHandler_{0.04, false};
	};

}

#endif
