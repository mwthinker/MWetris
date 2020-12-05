#ifndef MWETRIS_GAME_LOCALPLAYER_H
#define MWETRIS_GAME_LOCALPLAYER_H

#include "player.h"
#include "actionhandler.h"
#include "device.h"

namespace mwetris::game {

	class LocalPlayer;
	using LocalPlayerPtr = std::shared_ptr<LocalPlayer>;

	class LocalPlayer : public Player {
	public:
		LocalPlayer(const tetris::TetrisBoard& board, DevicePtr device);

		~LocalPlayer() override;

		LocalPlayer(const LocalPlayer&) = delete;
		LocalPlayer& operator=(const LocalPlayer&) = delete;

		void update(double deltaTime);

		void addRow(int holes);

		void updatePoints(int points);

		void updateName(const std::string& name);

		void updateLevelUpCounter(int counter);

		void updateLevel(int level);

		void updateGameOverPosition(int gameOverPosition);

		void updateRestart();

		void updateGameOver();

		std::string getName() const override {
			return name_;
		}

		int getLevel() const override {
			return level_;
		}

		int getPoints() const override {
			return points_;
		}

		int getClearedRows() const override {
			return 0;
		}

		int getLevelUpCounter() const override {
			return levelUpCounter_;
		}

		int getGameOverPosition() const override {
			return gameOverPosition_;
		}

		DevicePtr getDevice() const override {
			return device_;
		}

		virtual float getGravityDownSpeed() const {
			return 1 + level_ * 0.5f;
		}

	private:
		void handleBoardEvent(tetris::BoardEvent boardEvent, int value) override;

		DevicePtr device_;
		std::string name_;
		int points_, level_, levelUpCounter_;
		int gameOverPosition_;

		ActionHandler gravityMove_, downHandler_, leftHandler_, rightHandler_, rotateHandler_, downGroundHandler_;
	};

}

#endif
