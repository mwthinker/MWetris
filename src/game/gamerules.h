#ifndef GAMERULES_H
#define GAMERULES_H

#include "localplayer.h"

#include <vector>

namespace tetris {

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
		void addPlayer(const LocalPlayerPtr& player);

		void applyRules(BoardEvent gameEvent, LocalPlayerPtr player);

		void handleGameOverEvent(LocalPlayerPtr player);

		void handleRowClearedEvent(LocalPlayerPtr player, int rows);

		void triggerPointEvent(LocalPlayerPtr player, int newPoints, int oldPoints);

		void triggerGameOverEvent(LocalPlayerPtr player);

		void triggerLevelUpEvent(LocalPlayerPtr player, int newLevel, int oldLevel);

		void addRowsToOpponents(LocalPlayerPtr player);

		static int generateNbrHoles(const PlayerPtr& player);

		inline bool isMultiplayerGame() const {
			return localPlayers_.size() > 1;
		}

		struct Pair {
			Pair() = default;

			Pair(const mw::signals::Connection& connection, const LocalPlayerPtr& player) :
				connection_(connection), player_(player) {
			}

			mw::signals::Connection connection_;
			LocalPlayerPtr player_;
		};

		std::vector<LocalPlayerPtr> localPlayers_;
		mw::Signal<TetrisGameEvent&>& gameEventSignal_;
		int nbrOfAlivePlayers_{};
		static const double HOLES_PERCENT;
		static const int LEVELUP_NBR;
	};

}

#endif // GAMERULES_H
