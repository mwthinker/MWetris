#include "gamerules.h"
#include "tetrisgameevent.h"

using namespace tetris;

const double GameRules::HOLES_PERCENT = 0.2f;
const int GameRules::LEVELUP_NBR = 3;

GameRules::GameRules(mw::Signal<TetrisGameEvent&>& gameEventSignal)
	: gameEventSignal_{gameEventSignal} {
}

void GameRules::createGame(const std::vector<LocalPlayerPtr>& players) {
	localPlayers_ = players;
	for (const auto& player : players) {
		if (player->isGameOver()) {
			++nbrOfAlivePlayers_;
		}
	}

	for (auto& player : players) {
		player->addGameEventListener([=](BoardEvent gameEvent, const TetrisBoard& board) {
			applyRules(gameEvent, player);
		});
	}
}

void GameRules::restartGame() {
	for (auto& player : localPlayers_) {
		player->updateRestart();
	}
	nbrOfAlivePlayers_ = localPlayers_.size();
}

void GameRules::addPlayer(const LocalPlayerPtr& player) {
	player->addGameEventListener([&](BoardEvent gameEvent, const TetrisBoard& board) {
		applyRules(gameEvent, player);
	});

	localPlayers_.push_back(player);
	++nbrOfAlivePlayers_;
}

void GameRules::applyRules(BoardEvent gameEvent, LocalPlayerPtr player) {
	switch (gameEvent) {
		case BoardEvent::ONE_ROW_REMOVED:
			handleRowClearedEvent(player, 1);
			break;
		case BoardEvent::TWO_ROW_REMOVED:
			handleRowClearedEvent(player, 2);
			break;
		case BoardEvent::THREE_ROW_REMOVED:
			handleRowClearedEvent(player, 3);
			break;
		case BoardEvent::FOUR_ROW_REMOVED:
			handleRowClearedEvent(player, 4);
			addRowsToOpponents(player);
			break;
		case BoardEvent::GAME_OVER:
			handleGameOverEvent(player);
			break;
	}
}

void GameRules::handleGameOverEvent(LocalPlayerPtr player) {
	if (nbrOfAlivePlayers_ > 0) {
		player->updateGameOverPosition(nbrOfAlivePlayers_);
		--nbrOfAlivePlayers_;

		triggerGameOverEvent(player);

		if (nbrOfAlivePlayers_ < 2) {
			for (auto& opponent : localPlayers_) {
				if (player != opponent && !opponent->isGameOver()) {
					opponent->updateGameOver();
				}
			}
		}
	}
}

void GameRules::handleRowClearedEvent(LocalPlayerPtr player, int rows) {
	if (isMultiplayerGame()) {
		for (auto& opponent : localPlayers_) {
			if (player != opponent && !opponent->isGameOver()) {
				opponent->updateLevelUpCounter(opponent->getLevelUpCounter() + rows);
			}
		}
	} else {
		player->updateLevelUpCounter(player->getLevelUpCounter() + rows);
		int level = ( player->getLevelUpCounter() / LEVELUP_NBR) + 1;
		if (player->getLevel() != level) {
			player->updateLevel(level);
		}

		int oldPoints = player->getPoints();
		player->updatePoints(oldPoints + level * rows * rows);
		triggerPointEvent(player, player->getPoints(), oldPoints);
	}
}

void GameRules::triggerPointEvent(LocalPlayerPtr player, int newPoints, int oldPoints) {
	PointsChange pointsChange(player, newPoints, oldPoints);
	gameEventSignal_(pointsChange);
}

void GameRules::triggerGameOverEvent(LocalPlayerPtr player) {
	GameOver gameOver(player);
	gameEventSignal_(gameOver);
}

void GameRules::triggerLevelUpEvent(LocalPlayerPtr player, int newLevel, int oldLevel) {
	LevelChange levelChange(player, newLevel, oldLevel);
	gameEventSignal_(levelChange);
}

void GameRules::addRowsToOpponents(LocalPlayerPtr player) {
	for (auto& opponent : localPlayers_) {
		if (player != opponent && !opponent->isGameOver()) {
			for (int i = 0; i < 2; ++i) {
				int holesPerRow = generateNbrHoles(player);
				opponent->addRow(holesPerRow);
			}
		}
	}
}

int GameRules::generateNbrHoles(const PlayerPtr& player) {
	int holes = (int) (player->getTetrisBoard().getColumns() * HOLES_PERCENT);
	if (holes < 1) {
		return holes;
	} else {
		return holes;
	}
}
