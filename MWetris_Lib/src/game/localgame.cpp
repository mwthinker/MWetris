#include "localgame.h"
#include "tetrisgameevent.h"

#include <memory>

namespace {

	constexpr int LevelUpNbr = 10;

}

namespace mwetris::game {

	void LocalGame::createGame(LocalPlayerPtr player) {
		player_ = std::move(player);
		connection_ = player_->gameboardEventUpdate.connect([this, p = player_](tetris::BoardEvent gameEvent, int value) {
			applyRules(gameEvent, value, p);
		});
	}

	void LocalGame::restartGame() {
		player_->updateRestart();
	}

	LocalGame::LocalGame() {
	}

	void LocalGame::applyRules(tetris::BoardEvent gameEvent, int value, const LocalPlayerPtr& player) {
		if (tetris::BoardEvent::RowsRemoved == gameEvent) {
			rows_ += value;
			level_ = rows_ / LevelUpNbr + 1;
			points_ += level_ * value * value;

			player->updateLevel(level_);
			player->updatePoints(points_);
		}
	}

}
