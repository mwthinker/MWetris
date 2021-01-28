#ifndef MWETRIS_GAME_GAMERULES_H
#define MWETRIS_GAME_GAMERULES_H

#include "localplayer.h"
#include "tetrisgameevent.h"

#include <vector>

namespace mwetris::game {

	class GameRules {
	public:
		GameRules();

		void createGame(const std::vector<LocalPlayerPtr>& players);

		void restartGame();

		int getNbrAlivePlayers() const {
			return nbrOfAlivePlayers_;
		}

	private:
		void applyRules(tetris::BoardEvent gameEvent, int value, const LocalPlayerPtr& player);

		void handleGameOverEvent(const LocalPlayerPtr& player);

		void handleRowsRemovedEvent(const LocalPlayerPtr& player, int rows);

		void triggerPointEvent(const LocalPlayerPtr& player, int newPoints, int oldPoints);

		void triggerGameOverEvent(const LocalPlayerPtr& player);

		void triggerLevelUpEvent(const LocalPlayerPtr& player, int newLevel, int oldLevel);

		void addRowsToOpponents(const LocalPlayerPtr& player);

		bool isMultiplayerGame() const {
			return localPlayers_.size() > 1;
		}

		std::vector<LocalPlayerPtr> localPlayers_;
		int nbrOfAlivePlayers_{};
		mw::signals::ScopedConnections connections_;
	};

}

#endif
