#include "protocol.h"

#include "util.h"

namespace mwetris::network {

	/*

	void toTpSlot(const game::PlayerSlot& playerSlot, tp::Slot& tpSlot) {
		tpSlot.set_slot_type(tp::SlotType::UNSPECIFIED_SLOT_TYPE);

		std::visit([&](auto&& slot) mutable {
			using T = std::decay_t<decltype(slot)>;
			if constexpr (std::is_same_v<T, game::Human>) {
				tpSlot.set_slot_type(tp::SlotType::HUMAN);
				tpSlot.set_name(slot.name);
			} else if constexpr (std::is_same_v<T, game::Ai>) {
				tpSlot.set_slot_type(tp::SlotType::AI);
				tpSlot.set_name(slot.name);
			} else if constexpr (std::is_same_v<T, game::Remote>) {
				tpSlot.set_slot_type(tp::SlotType::REMOTE);
			} else if constexpr (std::is_same_v<T, game::OpenSlot>) {
				tpSlot.set_slot_type(tp::SlotType::OPEN_SLOT);
			} else if constexpr (std::is_same_v<T, game::ClosedSlot>) {
				tpSlot.set_slot_type(tp::SlotType::CLOSED_SLOT);
			} else {
				static_assert(always_false_v<T>, "non-exhaustive visitor!");
			}
		}, playerSlot);
	}

	void fromTpSlot(const std::string& uuid, const tp::Slot& tpSlot, game::PlayerSlot& playerSlot) {
		switch (tpSlot.slot_type()) {
		case tp::SlotType::HUMAN:
			playerSlot = game::Human{ tpSlot.name() };
			break;
		case tp::SlotType::AI:
			playerSlot = game::Ai{ tpSlot.name() };
			break;
		case tp::SlotType::REMOTE:
			playerSlot = game::Remote{};
			break;
		case tp::SlotType::OPEN_SLOT:
			playerSlot = game::OpenSlot{};
			break;
		case tp::SlotType::CLOSED_SLOT:
			playerSlot = game::ClosedSlot{};
			break;
		default:
			throw std::runtime_error("invalid slot type");
		}
	}
	*/
}
