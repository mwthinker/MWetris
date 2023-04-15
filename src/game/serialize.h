#ifndef MWETRIS_GAME_SERIALIZE_H
#define MWETRIS_GAME_SERIALIZE_H

#include "localplayer.h"

namespace mwetris::game {

	void saveGame(const std::vector<LocalPlayerPtr>& players);

	std::vector<LocalPlayerPtr> loadGame(const std::vector<DevicePtr>& availableDevices);

}

#endif
