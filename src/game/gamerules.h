#ifndef MWETRIS_GAME_GAMERULES_H
#define MWETRIS_GAME_GAMERULES_H

#include "localplayer.h"
#include "tetrisgameevent.h"

#include <vector>
#include <unordered_map>

namespace mwetris::game {

	class GameRules {
	public:
		GameRules();

		void createGame(const std::vector<LocalPlayerPtr>& players);

		void restartGame();

	private:
		struct PlayerData {
			int points;
			int levelUpCounter;
			int level;
		};

		void applyRules(tetris::BoardEvent gameEvent, int value, const LocalPlayerPtr& player);

		void handleGameOverEvent(const LocalPlayerPtr& player);

		void handleRowsRemovedEvent(const LocalPlayerPtr& player, int rows);

		void addRowsToOpponents(const LocalPlayerPtr& player);

		bool isMultiplayerGame() const {
			return localPlayers_.size() > 1;
		}

		std::unordered_map<LocalPlayerPtr, PlayerData> localPlayers_;
		int nbrAlivePlayers_ = 0;
		mw::signals::ScopedConnections connections_;
	};

}

#endif
