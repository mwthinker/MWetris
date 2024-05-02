#ifndef NETWORK_TESTUTIL_H
#define NETWORK_TESTUTIL_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <network/network.h>
#include <network/networkevent.h>

#include <client_to_server.pb.h>
#include <server_to_client.pb.h>

namespace mwetris::network {

	ProtobufMessage createMessage(const google::protobuf::MessageLite& wrapper);

	void addPlayerSlot(tp_s2c::GameLooby& gameLooby, const tp_s2c::GameLooby_SlotType slotType, const std::string& clientUuid, const std::string& name, bool ai = false);

	void expectEqual(const PlayerSlotEvent& playerSlotEvent, int index, const game::Human& remote);

	void expectEqual(const PlayerSlotEvent& playerSlotEvent, int index, const game::Ai& ai);

	void expectEqual(const PlayerSlotEvent& playerSlotEvent, int index, const game::Remote& remote);

	void expectEqual(const PlayerSlotEvent& playerSlotEvent, int index, const game::OpenSlot& openSlot);

	void expectEqual(const PlayerSlotEvent& playerSlotEvent, int index, const game::ClosedSlot& closedSlot);

}

#endif
