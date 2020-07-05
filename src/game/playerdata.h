#ifndef MWETRIS_GAME_PLAYERDATA_H
#define MWETRIS_GAME_PLAYERDATA_H

#include "device.h"
#include "block.h"

#include <string>
#include <vector>

namespace tetris::game {

	struct PlayerData {
		DevicePtr device_{};
		std::string name_;
		std::string deviceName_;
		bool ai_{};
		int points_{}, level_{}, levelUpCounter_{}, clearedRows_{};
		Block current_;
		BlockType next_{BlockType::Empty};
		std::vector<BlockType> board_;
		int lastPosition_{-1};
	};

}

#endif
