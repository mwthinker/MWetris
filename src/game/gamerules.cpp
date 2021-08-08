#include "gamerules.h"
#include "tetrisgameevent.h"

namespace mwetris::game {

	namespace {

		constexpr double HolesPercent = 0.2;
		constexpr int LevelUpNbr = 3;

		int generateNbrHoles(const Player& player) {
			auto holes = static_cast<int>(player.getTetrisBoard().getColumns() * HolesPercent);
			if (holes < 1) {
				return 1;
			}
			return holes;
		}

	}

	GameRules::GameRules() {
	}

	void GameRules::createGame(const std::vector<LocalPlayerPtr>& players) {
		connections_.clear();

		localPlayers_ = players;
		for (const auto& player : players) {
			connections_ += player->gameboardEventUpdate.connect([this, player](tetris::BoardEvent gameEvent, int value) {
				applyRules(gameEvent, value, player);
			});

			if (player->isGameOver()) {
				++nbrAlivePlayers_;
			}
		}
	}

	void GameRules::restartGame() {
		for (auto& player : localPlayers_) {
			player->updateRestart();
		}
		nbrAlivePlayers_ = static_cast<int>(localPlayers_.size());
	}

	void GameRules::applyRules(tetris::BoardEvent gameEvent, int value, const LocalPlayerPtr& player) {
		switch (gameEvent) {
			case tetris::BoardEvent::RowsRemoved:
				handleRowsRemovedEvent(player, 1);
				break;
			case tetris::BoardEvent::GameOver:
				handleGameOverEvent(player);
				break;
		}
	}

	void GameRules::handleGameOverEvent(const LocalPlayerPtr& player) {
		if (nbrAlivePlayers_ > 0) {
			player->updateGameOverPosition(nbrAlivePlayers_);
			--nbrAlivePlayers_;

			triggerGameOverEvent(player);

			if (nbrAlivePlayers_ < 2) {
				for (auto& opponent : localPlayers_) {
					if (player != opponent && !opponent->isGameOver()) {
						opponent->updateGameOver();
					}
				}
			}
		}
	}

	void GameRules::handleRowsRemovedEvent(const LocalPlayerPtr& player, int rows) {
		if (isMultiplayerGame()) {
			for (auto& opponent : localPlayers_) {
				if (player != opponent && !opponent->isGameOver()) {
					//opponent->updateLevelUpCounter(opponent->getLevelUpCounter() + rows);
					
					if (rows == 4) {
						addRowsToOpponents(player);
					}
				}
			}
		} else {
			//player->updateLevelUpCounter(player->getLevelUpCounter() + rows);
			int level = 1;// = (player->getLevelUpCounter() / LevelUpNbr) + 1;
			if (player->getLevel() != level) {
				player->updateLevel(level);
			}
			
			int oldPoints = player->getPoints();
			player->updatePoints(oldPoints + level * rows * rows);
			triggerPointEvent(player, player->getPoints(), oldPoints);
		}
	}

	void GameRules::triggerPointEvent(const LocalPlayerPtr& player, int newPoints, int oldPoints) {
		PointsChange pointsChange(player, newPoints, oldPoints);
		//gameEventSignal_(pointsChange);
	}

	void GameRules::triggerGameOverEvent(const LocalPlayerPtr& player) {
		GameOver gameOver(player);
		//gameEventSignal_(gameOver);
	}

	void GameRules::triggerLevelUpEvent(const LocalPlayerPtr& player, int newLevel, int oldLevel) {
		LevelChange levelChange(player, newLevel, oldLevel);
		//gameEventSignal_(levelChange);
	}

	void GameRules::addRowsToOpponents(const LocalPlayerPtr& player) {
		for (auto& opponent : localPlayers_) {
			if (player != opponent && !opponent->isGameOver()) {
				for (int i = 0; i < 2; ++i) {
					int holesPerRow = generateNbrHoles(*player);
					opponent->addRow(holesPerRow);
				}
			}
		}
	}

}
