#include "testutil.h"

using namespace ::testing;

namespace mwetris::network {

	void addPlayerSlot(tp_s2c::GameLooby& gameLooby, const tp_s2c::GameLooby_SlotType slotType, const ClientId& clientUuid, const std::string& name, bool ai) {
		auto slot = gameLooby.add_slots();
		slot->set_ai(ai);
		setTp(clientUuid, *slot->mutable_client_id());
		slot->set_name(name);
		slot->set_slot_type(slotType);
	}

	void expectEqual(const PlayerSlotEvent& playerSlotEvent, int index, const game::Human& remote) {
		EXPECT_EQ(playerSlotEvent.index, index);
		if (auto playerSlot = std::get_if<game::Human>(&playerSlotEvent.playerSlot)) {
			EXPECT_EQ(playerSlot->device, remote.device);
			EXPECT_EQ(playerSlot->name, remote.name);
		} else {
			FAIL() << "playerSlotEvent.playerSlot is not of type game::Human";
		}
	}

	void expectEqual(const PlayerSlotEvent& playerSlotEvent, int index, const game::Ai& ai) {
		EXPECT_EQ(playerSlotEvent.index, index);
		if (auto playerSlot = std::get_if<game::Ai>(&playerSlotEvent.playerSlot)) {
			EXPECT_EQ(playerSlot->name, ai.name);
		} else {
			FAIL() << "playerSlotEvent.playerSlot is not of type game::Remote";
		}
	}

	void expectEqual(const PlayerSlotEvent& playerSlotEvent, int index, const game::Remote& remote) {
		EXPECT_EQ(playerSlotEvent.index, index);
		if (auto playerSlot = std::get_if<game::Remote>(&playerSlotEvent.playerSlot)) {
			EXPECT_EQ(playerSlot->ai, remote.ai);
			EXPECT_EQ(playerSlot->name, remote.name);
		} else {
			FAIL() << "playerSlotEvent.playerSlot is not of type game::Remote";
		}
	}

	void expectEqual(const PlayerSlotEvent& playerSlotEvent, int index, const game::OpenSlot& openSlot) {
		EXPECT_EQ(playerSlotEvent.index, index);
		EXPECT_TRUE(std::holds_alternative<game::OpenSlot>(playerSlotEvent.playerSlot)) << "playerSlotEvent.playerSlot is not of type game::OpenSlot";
	}

	void expectEqual(const PlayerSlotEvent& playerSlotEvent, int index, const game::ClosedSlot& closedSlot) {
		EXPECT_EQ(playerSlotEvent.index, index);
		EXPECT_TRUE(std::holds_alternative<game::ClosedSlot>(playerSlotEvent.playerSlot)) << "playerSlotEvent.playerSlot is not of type game::ClosedSlot";
	}

}
