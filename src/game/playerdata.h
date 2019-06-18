#ifndef PLAYERDATA_H
#define PLAYERDATA_H

#include "device.h"
#include "block.h"

#include <string>
#include <vector>

namespace tetris {

	class PlayerData {
	public:
		PlayerData() : device_(nullptr), name_(""), deviceName_(""),
			ai_(false), points_(0), level_(0), levelUpCounter_(0), clearedRows_(0),
			lastPosition_(-1), next_(BlockType::EMPTY) {
		}

		IDevicePtr device_;
		std::string name_;
		std::string deviceName_;
		bool ai_;
		int points_, level_, levelUpCounter_, clearedRows_;
		Block current_;
		BlockType next_;
		std::vector<BlockType> board_;
		int lastPosition_;
	};

}

#endif // PLAYERDATA_H
