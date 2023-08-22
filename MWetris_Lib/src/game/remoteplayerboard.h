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

		int getClearedRows() const override {
			return clearedRows_;
		}

		const PlayerData& getPlayerData() const override {
			return playerData_;
		}

	private:
		void handleBoardEvent(tetris::BoardEvent boardEvent, int value) override;

		int clearedRows_ = 0;
		PlayerData playerData_{};
	};

}

#endif
