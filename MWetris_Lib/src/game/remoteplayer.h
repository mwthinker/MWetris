#ifndef MWETRIS_GAME_REMOTEPLAYER_H
#define MWETRIS_GAME_REMOTEPLAYER_H

#include "remoteplayerboard.h"
#include "tetrisparameters.h"

#include <tetrisboard.h>

#include <mw/signal.h>

#include <memory>

namespace mwetris::game {

	class RemotePlayer;
	using RemotePlayerPtr = std::shared_ptr<RemotePlayer>;

	class RemotePlayer {
	public:
		RemotePlayer() {
			tetris::TetrisBoard tetrisBoard{TetrisWidth, TetrisHeight, tetris::BlockType::I, tetris::BlockType::I};
			remotePlayerBoard_ = std::make_shared<RemotePlayerBoard>(tetrisBoard, "RemotePlayer");
		}

		void update(double timeStep) {
			//remotePlayerBoard_->update(Input{}, deltaTime);
		}

		void updateRestart() {}

		RemotePlayerBoardPtr getPlayerBoard() const {
			return remotePlayerBoard_;
		}

		[[nodiscard]]
		mw::signals::Connection addEventCallback(std::function<void(tetris::BoardEvent, int)>&& callback) {
			return {};
		}

	private:
		RemotePlayerBoardPtr remotePlayerBoard_;

	};

}

#endif
