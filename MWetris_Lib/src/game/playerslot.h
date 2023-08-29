#ifndef MWETRIS_GAME_HUMANAI_H
#define MWETRIS_GAME_HUMANAI_H

#include "device.h"

#include <ai.h>

#include <variant>

namespace mwetris::game {

	struct Human {
		std::string name;
		DevicePtr device;
	};

	struct Ai {
		std::string name;
		tetris::Ai ai;
	};

	struct Remote {
		std::string name;
		bool ai = false;
	};

	struct OpenSlot {
	};

	struct ClosedSlot {
	};

	using PlayerSlot = std::variant<Human, Ai, Remote, OpenSlot, ClosedSlot>;
}

#endif
