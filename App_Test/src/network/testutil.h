#ifndef NETWORK_TESTUTIL_H
#define NETWORK_TESTUTIL_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <mwetris/cnetwork/network.h>
#include <mwetris/cnetwork/networkevent.h>

#include <mwetris/game/playerslot.h>

#include <protocol/client_to_server.pb.h>
#include <protocol/server_to_client.pb.h>

#include <asio.hpp>

namespace network {

	void addPlayerSlot(tp_s2c::GameLooby& gameLooby, const tp_s2c::GameLooby_SlotType slotType, const ClientId& clientUuid, const std::string& name, bool ai = false);

	void expectEqual(const mwetris::cnetwork::PlayerSlotEvent& playerSlotEvent, int index, const mwetris::game::Human& remote);

	void expectEqual(const mwetris::cnetwork::PlayerSlotEvent& playerSlotEvent, int index, const mwetris::game::Ai& ai);

	void expectEqual(const mwetris::cnetwork::PlayerSlotEvent& playerSlotEvent, int index, const mwetris::game::Remote& remote);

	void expectEqual(const mwetris::cnetwork::PlayerSlotEvent& playerSlotEvent, int index, const mwetris::game::OpenSlot& openSlot);

	void expectEqual(const mwetris::cnetwork::PlayerSlotEvent& playerSlotEvent, int index, const mwetris::game::ClosedSlot& closedSlot);

}

#endif
