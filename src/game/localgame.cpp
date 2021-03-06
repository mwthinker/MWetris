#include "localgame.h"
#include "gamerules.h"
#include "tetrisgameevent.h"

#include <memory>

namespace mwetris::game {

	void LocalGame::createGame(const std::vector<LocalPlayerPtr>& players) {
		gameRules_->createGame(players);
	}

	void LocalGame::restartGame() {
		gameRules_->restartGame();
	}

	LocalGame::LocalGame()
		: gameRules_{std::make_unique<GameRules>()} {
	}

}
