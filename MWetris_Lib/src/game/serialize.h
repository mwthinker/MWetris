#ifndef MWETRIS_GAME_SERIALIZE_H
#define MWETRIS_GAME_SERIALIZE_H

#include "localplayerboard.h"

#include <chrono>

namespace mwetris::game {

	bool hasSavedGame();

	void clearSavedGame();

	void saveGame(const PlayerBoard& playerBoard);

	LocalPlayerBoardPtr loadGame();

}

#endif
