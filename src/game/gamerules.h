#ifndef GAMERULES_H
#define GAMERULES_H

#include "localplayer.h"

#include <vector>

namespace tetris {

	class TetrisGameEvent;

	class GameRules {
	public:
		GameRules(mw::Signal<TetrisGameEvent&>& gameEventSignal);

		void createGame(const std::vector<ILocalPlayerPtr>& players);

		void restartGame();

		int getNbrAlivePlayers() const {
			return nbrOfAlivePlayers_;
		}

	private:
		void addPlayer(const ILocalPlayerPtr& player);

		void applyRules(BoardEvent gameEvent, ILocalPlayerPtr player);

		void handleGameOverEvent(ILocalPlayerPtr player);

		void handleRowClearedEvent(ILocalPlayerPtr player, int rows);

		void triggerPointEvent(ILocalPlayerPtr player, int newPoints, int oldPoints);

		void triggerGameOverEvent(ILocalPlayerPtr player);

		void triggerLevelUpEvent(ILocalPlayerPtr player, int newLevel, int oldLevel);

		void addRowsToOpponents(ILocalPlayerPtr player);

		static int generateNbrHoles(const IPlayerPtr& player);

		inline bool isMultiplayerGame() const {
			return localPlayers_.size() > 1;
		}

		struct Pair {
			Pair() = default;

			Pair(const mw::signals::Connection& connection, const ILocalPlayerPtr& player) :
				connection_(connection), player_(player) {
			}

			mw::signals::Connection connection_;
			ILocalPlayerPtr player_;
		};

		std::vector<ILocalPlayerPtr> localPlayers_;
		mw::Signal<TetrisGameEvent&>& gameEventSignal_;
		int nbrOfAlivePlayers_;
		static const double HOLES_PERCENT;
		static const int LEVELUP_NBR;
	};

}

#endif // GAMERULES_H
