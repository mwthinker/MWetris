#ifndef APP_GAME_SERIALIZE_H
#define APP_GAME_SERIALIZE_H

#include "device.h"

#include <memory>

namespace app::game {
	
	class Player;
	using PlayerPtr = std::shared_ptr<Player>;

	bool hasSavedGame();

	void clearSavedGame();

	void saveGame(const Player& player);

	PlayerPtr loadGame(DevicePtr device);

}

#endif
