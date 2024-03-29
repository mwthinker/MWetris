#include "protocol.h"

#include "util.h"

namespace mwetris::network {

	void toTpSlot(const game::PlayerSlot& playerSlot, tp_c2s::Slot& tpSlot) {
		tpSlot.set_slot_type(tp_c2s::SlotType::UNSPECIFIED_SLOT_TYPE);

		std::visit([&](auto&& slot) mutable {
			using T = std::decay_t<decltype(slot)>;
			if constexpr (std::is_same_v<T, game::Human>) {
				tpSlot.set_slot_type(tp_c2s::SlotType::HUMAN);
				tpSlot.set_name(slot.name);
			} else if constexpr (std::is_same_v<T, game::Ai>) {
				tpSlot.set_slot_type(tp_c2s::SlotType::AI);
				tpSlot.set_name(slot.name);
			} else if constexpr (std::is_same_v<T, game::Remote>) {
				tpSlot.set_slot_type(tp_c2s::SlotType::REMOTE);
			} else if constexpr (std::is_same_v<T, game::OpenSlot>) {
				tpSlot.set_slot_type(tp_c2s::SlotType::OPEN_SLOT);
			} else if constexpr (std::is_same_v<T, game::ClosedSlot>) {
				tpSlot.set_slot_type(tp_c2s::SlotType::CLOSED_SLOT);
			} else {
				static_assert(always_false_v<T>, "non-exhaustive visitor!");
			}
		}, playerSlot);
	}

}
