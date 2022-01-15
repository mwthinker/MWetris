#include "gamerules.h"
#include "tetrisgameevent.h"

#include <ranges>

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

		for (const auto& player : players) {
			localPlayers_[player] = PlayerData{};
			connections_ += player->gameboardEventUpdate.connect([this, player](tetris::BoardEvent gameEvent, int value) {
				applyRules(gameEvent, value, player);
			});
			if (player->isGameOver()) {
				++nbrAlivePlayers_;
			}
		}
	}

	void GameRules::restartGame() {
		for (auto& [player, _] : localPlayers_) {
			player->updateRestart();
		}
		nbrAlivePlayers_ = static_cast<int>(localPlayers_.size());
	}

	void GameRules::applyRules(tetris::BoardEvent gameEvent, int value, const LocalPlayerPtr& player) {
		switch (gameEvent) {
			case tetris::BoardEvent::BlockCollision:
				if (isMultiplayerGame()) {
					localPlayers_[player].level = localPlayers_[player].levelUpCounter / LevelUpNbr + 1;
					player->updateLevel(localPlayers_[player].level);
				}
				break;
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

			if (nbrAlivePlayers_ < 2) {
				for (auto& [opponent, _] : localPlayers_) {
					if (player != opponent && !opponent->isGameOver()) {
						opponent->updateGameOver();
					}
				}
			}
		}
	}

	void GameRules::handleRowsRemovedEvent(const LocalPlayerPtr& player, int rows) {
		if (isMultiplayerGame()) {
			for (auto& [opponent, _] : localPlayers_) {
				if (player != opponent && !opponent->isGameOver()) {
					auto& data = localPlayers_[opponent];
					data.levelUpCounter += rows;
					
					if (rows == 4) {
						addRowsToOpponents(player);
					}
				}
			}
		} else {
			auto& data = localPlayers_[player];
			data.levelUpCounter += rows;;
			data.level = data.levelUpCounter / LevelUpNbr + 1;
			player->updateLevel(data.level);
			
			data.points += data.level * rows * rows;
			player->updatePoints(data.points);
		}
	}

	void GameRules::addRowsToOpponents(const LocalPlayerPtr& player) {
		for (auto& [opponent, _] : localPlayers_) {
			if (player != opponent && !opponent->isGameOver()) {
				for (int i = 0; i < 2; ++i) {
					int holesPerRow = generateNbrHoles(*player);
					opponent->addRow(holesPerRow);
				}
			}
		}
	}

}
