#ifndef NETWORK_TESTUTIL_H
#define NETWORK_TESTUTIL_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <app/cnetwork/network.h>
#include <app/cnetwork/networkevent.h>

#include <app/game/playerslot.h>

#include <protocol/client_to_server.pb.h>
#include <protocol/server_to_client.pb.h>

#include <asio.hpp>

namespace network {

	void addPlayerSlot(tp_s2c::GameLooby& gameLooby, const tp_s2c::GameLooby_SlotType slotType, const ClientId& clientUuid, const std::string& name, bool ai = false);

	void expectEqual(const app::cnetwork::PlayerSlotEvent& playerSlotEvent, int index, const app::game::Human& remote);

	void expectEqual(const app::cnetwork::PlayerSlotEvent& playerSlotEvent, int index, const app::game::Ai& ai);

	void expectEqual(const app::cnetwork::PlayerSlotEvent& playerSlotEvent, int index, const app::game::Remote& remote);

	void expectEqual(const app::cnetwork::PlayerSlotEvent& playerSlotEvent, int index, const app::game::OpenSlot& openSlot);

	void expectEqual(const app::cnetwork::PlayerSlotEvent& playerSlotEvent, int index, const app::game::ClosedSlot& closedSlot);

}

#endif
