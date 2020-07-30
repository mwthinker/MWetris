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

	GameRules::GameRules(std::shared_ptr<EventManager> eventManager)
		: eventManager_{eventManager} {

	}

	void GameRules::createGame(const std::vector<LocalPlayerPtr>& players) {
		localPlayers_ = players;
		for (const auto& player : players) {
			if (player->isGameOver()) {
				++nbrOfAlivePlayers_;
			}
		}

		for (auto& player : players) {
			eventManager_->subscribe(player->getSenderId(), [this](EventPtr event) {
				handleEvent(*event);
			});
		}
	}

	void GameRules::handleEvent(Event& event) {
		try {
			auto& countDown = dynamic_cast<game::CountDown&>(event);
		} catch (std::bad_cast&) {}
	}

	void GameRules::restartGame() {
		for (auto& player : localPlayers_) {
			player->updateRestart();
		}
		nbrOfAlivePlayers_ = static_cast<int>(localPlayers_.size());
	}

	void GameRules::applyRules(tetris::BoardEvent gameEvent, const LocalPlayerPtr& player) {
		switch (gameEvent) {
			case tetris::BoardEvent::RowsRemoved:
				handleRowClearedEvent(player, 1);
				break;
			/*
			case BoardEvent::TwoRowRemoved:
				handleRowClearedEvent(player, 2);
				break;
			case BoardEvent::ThreeRowRemoved:
				handleRowClearedEvent(player, 3);
				break;
			case BoardEvent::FourRowRemoved:
				handleRowClearedEvent(player, 4);
				addRowsToOpponents(player);
				break;
			*/
			case tetris::BoardEvent::GameOver:
				handleGameOverEvent(player);
				break;
		}
	}

	void GameRules::handleGameOverEvent(const LocalPlayerPtr& player) {
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

	void GameRules::handleRowClearedEvent(const LocalPlayerPtr& player, int rows) {
		if (isMultiplayerGame()) {
			for (auto& opponent : localPlayers_) {
				if (player != opponent && !opponent->isGameOver()) {
					opponent->updateLevelUpCounter(opponent->getLevelUpCounter() + rows);
				}
			}
		} else {
			player->updateLevelUpCounter(player->getLevelUpCounter() + rows);
			int level = (player->getLevelUpCounter() / LevelUpNbr) + 1;
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
