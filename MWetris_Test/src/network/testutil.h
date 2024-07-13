#ifndef NETWORK_TESTUTIL_H
#define NETWORK_TESTUTIL_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <mwetris/network/network.h>
#include <mwetris/network/networkevent.h>

#include <protocol/client_to_server.pb.h>
#include <protocol/server_to_client.pb.h>

#include <asio.hpp>

namespace mwetris::network {

	void addPlayerSlot(tp_s2c::GameLooby& gameLooby, const tp_s2c::GameLooby_SlotType slotType, const ClientId& clientUuid, const std::string& name, bool ai = false);

	void expectEqual(const PlayerSlotEvent& playerSlotEvent, int index, const game::Human& remote);

	void expectEqual(const PlayerSlotEvent& playerSlotEvent, int index, const game::Ai& ai);

	void expectEqual(const PlayerSlotEvent& playerSlotEvent, int index, const game::Remote& remote);

	void expectEqual(const PlayerSlotEvent& playerSlotEvent, int index, const game::OpenSlot& openSlot);

	void expectEqual(const PlayerSlotEvent& playerSlotEvent, int index, const game::ClosedSlot& closedSlot);

}

#endif
