#ifndef MWETRIS_GAME_GAMERULES_H
#define MWETRIS_GAME_GAMERULES_H

#include "localplayer.h"
#include "eventmanager.h"

#include <vector>

namespace tetris::game {

	class TetrisGameEvent;

	class GameRules {
	public:
		GameRules(std::shared_ptr<EventManager> eventManager);

		void createGame(const std::vector<LocalPlayerPtr>& players);

		void restartGame();

		int getNbrAlivePlayers() const {
			return nbrOfAlivePlayers_;
		}

	private:
		void handleEvent(Event& event);

		void addPlayer(LocalPlayerPtr player);

		void applyRules(BoardEvent gameEvent, const LocalPlayerPtr& player);

		void handleGameOverEvent(const LocalPlayerPtr& player);

		void handleRowClearedEvent(const LocalPlayerPtr& player, int rows);

		void triggerPointEvent(const LocalPlayerPtr& player, int newPoints, int oldPoints);

		void triggerGameOverEvent(const LocalPlayerPtr& player);

		void triggerLevelUpEvent(const LocalPlayerPtr& player, int newLevel, int oldLevel);

		void addRowsToOpponents(const LocalPlayerPtr& player);

		bool isMultiplayerGame() const {
			return localPlayers_.size() > 1;
		}

		std::shared_ptr<EventManager> eventManager_;
		std::vector<LocalPlayerPtr> localPlayers_;
		int nbrOfAlivePlayers_{};
	};

}

#endif
