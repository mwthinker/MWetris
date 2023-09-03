#ifndef MWETRIS_GAME_LOCALPLAYERBOARD_H
#define MWETRIS_GAME_LOCALPLAYERBOARD_H

#include "playerboard.h"
#include "actionhandler.h"

namespace mwetris::game {

	class LocalPlayerBoard;
	using LocalPlayerBoardPtr = std::shared_ptr<LocalPlayerBoard>;

	class LocalPlayerBoard : public PlayerBoard {
	public:
		mw::PublicSignal<LocalPlayerBoard, PlayerBoardEvent> playerBoardUpdate;

		LocalPlayerBoard(const tetris::TetrisBoard& board, const std::string& name);

		LocalPlayerBoard(const LocalPlayerBoard&) = delete;
		LocalPlayerBoard& operator=(const LocalPlayerBoard&) = delete;
		LocalPlayerBoard(LocalPlayerBoard&&) = delete;
		LocalPlayerBoard& operator=(LocalPlayerBoard&&) = delete;

		void update(Input input, double deltaTime);

		void addRow(int holes);

		void updateName(const std::string& name);

		void updateClearedRows(int clearedRows);

		void updateRestart(tetris::BlockType current, tetris::BlockType next);

		void updateGameOver();

		void updatePlayerData(const PlayerData& playerData);

		const PlayerData& getPlayerData() const override {
			return playerData_;
		}

		int getClearedRows() const override {
			return clearedRows_;
		}

		void updateGravity(float speed) {
			speed_ = speed;
		}

	private:
		void update(tetris::Move move);

		void handleBoardEvent(tetris::BoardEvent boardEvent, int value) override;

		int clearedRows_ = 0;

		ActionHandler leftHandler_{0.09, false};
		ActionHandler rightHandler_{0.09, false};
		ActionHandler rotateHandler_{0.0, true};
		ActionHandler downGroundHandler_{0.0, true};
		ActionHandler gravityMove_{1, false};  // Value doesn't matter! Changes every frame.
		ActionHandler downHandler_{0.04, false};

		float speed_ = 1.f;
		PlayerData playerData_;
		std::vector<tetris::BlockType> externalRows_;
	};

}

#endif
