#ifndef MWETRIS_GAME_SNAPSHOT_H
#define MWETRIS_GAME_SNAPSHOT_H

#include "device.h"
#include "localplayer.h"

namespace mwetris::game {

	struct PlayerDevice {
		DevicePtr device;
		LocalPlayerBoardPtr playerBoard;
	};

}

#endif
