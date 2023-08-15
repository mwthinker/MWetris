#ifndef MWETRIS_GAME_AIPLAYER_H
#define MWETRIS_GAME_AiPLAYER_H

#include "player.h"
#include "localplayerboard.h"

#include "computer.h"

#include <ai.h>

namespace mwetris::game {

	class AiPlayer : public Player {
	public:
		AiPlayer(const tetris::Ai& ai, LocalPlayerBoardPtr localPlayerBoard)
			: computer_{ai}
			, localPlayerBoard_{localPlayerBoard} {

			connections_ += localPlayerBoard_->gameboardEventUpdate.connect([this](tetris::BoardEvent boardEvent, int nbr) {
				computer_.onGameboardEvent(localPlayerBoard_->getTetrisBoard(), boardEvent, nbr);
			});
		}

		void update(double deltaTime) override {
			localPlayerBoard_->update(computer_.getInput(), deltaTime);
		}

		[[nodiscard]]
		mw::signals::Connection addEventCallback(std::function<void(tetris::BoardEvent, int)>&& callback) override {
			return localPlayerBoard_->gameboardEventUpdate.connect(callback);
		}

		void updateRestart() override {
			localPlayerBoard_->updateRestart();
		}
		
		PlayerBoardPtr getPlayerBoard() const override {
			return localPlayerBoard_;
		}

	private:
		mw::signals::ScopedConnections connections_;

		Computer computer_;
		LocalPlayerBoardPtr localPlayerBoard_;
	};

}

#endif
