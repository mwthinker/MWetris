#include "localgame.h"
#include "gamerules.h"
#include "tetrisgameevent.h"

#include <memory>

namespace tetris::game {

	void LocalGame::createGame(const std::vector<LocalPlayerPtr>& players) {
		gameRules_->createGame(players);
	}

	void LocalGame::restartGame() {
		gameRules_->restartGame();
	}

	int LocalGame::getNbrAlivePlayers() const {
		return gameRules_->getNbrAlivePlayers();
	}

	LocalGame::LocalGame(mw::Signal<TetrisGameEvent&>& gameEventSignal)
		: gameRules_{std::make_unique<GameRules>(gameEventSignal)} {
	}

}
