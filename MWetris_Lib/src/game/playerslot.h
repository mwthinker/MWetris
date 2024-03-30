#ifndef MWETRIS_GAME_HUMANAI_H
#define MWETRIS_GAME_HUMANAI_H

#include "device.h"

#include <ai.h>

#include <variant>
#include <vector>

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
		std::string uuid;
	};

	struct OpenSlot {
	};

	struct ClosedSlot {
	};

	using PlayerSlot = std::variant<Human, Ai, Remote, OpenSlot, ClosedSlot>;

	inline void reset(std::vector<game::PlayerSlot>& slots) {
		for (auto& slot : slots) {
			slot = game::OpenSlot{};
		}
	}

	template <typename Type>
	int count(const std::vector<game::PlayerSlot>& playerSlots) {
		int nbr = 0;
		for (const auto& playerSlot : playerSlots) {
			if (auto type = std::get_if<Type>(&playerSlot); type) {
				++nbr;
			}
		}
		return nbr;
	}

	inline int playersInSlots(const std::vector<game::PlayerSlot>& playerSlots) {
		return count<game::Human>(playerSlots) + count<game::Ai>(playerSlots) + count<game::Remote>(playerSlots);
	}

	template <typename Type>
	std::vector<Type> extract(const std::vector<game::PlayerSlot>& playerSlots) {
		std::vector<Type> types;
		for (const auto& playerSlot : playerSlots) {
			if (auto type = std::get_if<Type>(&playerSlot); type) {
				types.push_back(*type);
			}
		}
		return types;
	}
}

#endif
