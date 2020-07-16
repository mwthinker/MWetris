#ifndef MWETRIS_GAME_LOCALPLAYER_H
#define MWETRIS_GAME_LOCALPLAYER_H

#include "player.h"
#include "tetrisboardwrapper.h"
#include "actionhandler.h"
#include "device.h"
#include "playerdata.h"

namespace tetris::game {

	class LocalPlayer;
	using LocalPlayerPtr = std::shared_ptr<LocalPlayer>;

	class LocalPlayer : public Player {
	public:
		LocalPlayer(const TetrisBoard& board, const DevicePtr& device);

		virtual ~LocalPlayer();

		LocalPlayer(const LocalPlayer&) = delete;
		LocalPlayer& operator=(const LocalPlayer&) = delete;

		virtual void update(double deltaTime);

		virtual void addRow(int holes);

		virtual void updatePoints(int points);

		virtual void updateName(const std::string& name);

		virtual void updateLevelUpCounter(int counter);

		virtual void updateLevel(int level);

		virtual void updateGameOverPosition(int gameOverPosition);

		virtual void updateRestart();

		virtual void updateGameOver();

		mw::signals::Connection addGameEventListener(
			const std::function<void(BoardEvent, const TetrisBoardWrapper&)>& callback) override {
			return tetrisBoard_.addGameEventListener(callback);
		}

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
			return tetrisBoard_.getRemovedRows();
		}

		int getLevelUpCounter() const override {
			return levelUpCounter_;
		}

		bool isGameOver() const override {
			return tetrisBoard_.isGameOver();
		}

		int getGameOverPosition() const override {
			return gameOverPosition_;
		}

		DevicePtr getDevice() const override {
			return device_;
		}

		const TetrisBoardWrapper& getTetrisBoard() const override {
			return tetrisBoard_;
		}

		virtual float getGravityDownSpeed() const {
			return 1 + level_ * 0.5f;
		}

	private:
		void boardListener(BoardEvent gameEvent);

		DevicePtr device_;
		std::string name_;
		int points_, level_, levelUpCounter_;
		TetrisBoardWrapper tetrisBoard_;
		int gameOverPosition_;
		mw::signals::Connection connection_;

		ActionHandler gravityMove_, downHandler_, leftHandler_, rightHandler_, rotateHandler_, downGroundHandler_;
	};

}

#endif
