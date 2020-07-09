#ifndef MWETRIS_GAME_GAMERULES_H
#define MWETRIS_GAME_GAMERULES_H

#include "localplayer.h"

#include <vector>

namespace tetris::game {

	class TetrisGameEvent;

	class GameRules {
	public:
		GameRules(mw::Signal<TetrisGameEvent&>& gameEventSignal);

		void createGame(const std::vector<LocalPlayerPtr>& players);

		void restartGame();

		int getNbrAlivePlayers() const {
			return nbrOfAlivePlayers_;
		}

	private:
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

		std::vector<LocalPlayerPtr> localPlayers_;
		mw::Signal<TetrisGameEvent&>& gameEventSignal_;
		int nbrOfAlivePlayers_{};
	};

}

#endif
