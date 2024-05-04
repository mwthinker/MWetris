#ifndef MWETRIS_GAME_SERIALIZE_H
#define MWETRIS_GAME_SERIALIZE_H

#include "device.h"

#include <chrono>
#include <memory>

namespace mwetris::game {
	
	class Player;
	using PlayerPtr = std::shared_ptr<Player>;

	bool hasSavedGame();

	void clearSavedGame();

	void saveGame(const Player& player);

	PlayerPtr loadGame(DevicePtr device);

}

#endif
