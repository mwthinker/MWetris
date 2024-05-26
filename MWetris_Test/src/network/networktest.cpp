#include "testutil.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <network/client.h>
#include <network/network.h>
#include <network/id.h>
#include <network/debugclient.h>
#include <network/debugserver.h>

#include <shared.pb.h>
#include <client_to_server.pb.h>
#include <server_to_client.pb.h>
#include <asio.hpp>

using namespace ::testing;

namespace mwetris::network {

	class NetworkTest : public ::testing::Test {
	protected:
		NetworkTest() {
		}

		~NetworkTest() override {}

		void SetUp() override {
			auto debugServer = std::make_shared<DebugServer>(ioContext_);
			debugClientOnNetwork_ = std::dynamic_pointer_cast<DebugClientOnNetwork>(debugServer->addClient());
			network_ = std::make_shared<Network>(debugClientOnNetwork_);
			debugClientOnNetwork_->getIoContext().restart();
		}

		void TearDown() override {
			debugClientOnNetwork_ = nullptr;
			network_ = nullptr;
			ioContext_.stop();
			ioContext_.restart();
			wrapperFromServer.Clear();
		}

		void pollOne() {
			debugClientOnNetwork_->getIoContext().poll_one();
		}

		void mockReceiveGameRoomCreated(const GameRoomId& gameRoomId, const ClientId& clientUuid) {
			tp_s2c::Wrapper wrapperFromServer;
			auto gameRoomCreated = wrapperFromServer.mutable_game_room_created();
			setTp(clientUuid, *gameRoomCreated->mutable_client_id());
			setTp(gameRoomId, *gameRoomCreated->mutable_game_room_id());

			expectCallClientReceive(wrapperFromServer);
			wrapperFromServer.Clear();
		}

		void mockReceiveGameRoomJoined(const GameRoomId& gameRoomId, const ClientId& clientUuid) {
			tp_s2c::Wrapper wrapperFromServer;
			auto gameRoomJoined = wrapperFromServer.mutable_game_room_joined();
			setTp(clientUuid, *gameRoomJoined->mutable_client_id());
			setTp(gameRoomId, *gameRoomJoined->mutable_game_room_id());

			expectCallClientReceive(wrapperFromServer);
			wrapperFromServer.Clear();
		}

		void expectCallClientReceive(const tp_s2c::Wrapper& wrapper) {
			ProtobufMessage message;
			message.clear();
			message.setBuffer(wrapper);
			debugClientOnNetwork_->pushReceivedMessage(std::move(message));
		}

		tp_s2c::Wrapper wrapperFromServer;
		std::shared_ptr<DebugClientOnNetwork> debugClientOnNetwork_;
		std::shared_ptr<Network> network_;
		asio::io_context ioContext_;
	};

	TEST_F(NetworkTest, network_isNotInsideRoom) {
		ASSERT_FALSE(network_->isInsideRoom());
		ASSERT_EQ(network_->getGameRoomId(), GameRoomId{});
	}

	TEST_F(NetworkTest, network_createGameRoom) {
		// Given
		bool createGameRoomEventCalled = false;
		auto connection = network_->createGameRoomEvent.connect([&](const CreateGameRoomEvent& createGameRoomEvent) {
			createGameRoomEventCalled = true;
		});

		mockReceiveGameRoomCreated(GameRoomId{"server uuid"}, ClientId{"client uuid"});

		// When
		EXPECT_FALSE(network_->isInsideRoom());
		EXPECT_FALSE(createGameRoomEventCalled);
		EXPECT_NE(network_->getGameRoomId(), GameRoomId{"server uuid"});
		pollOne();

		// Then.
		EXPECT_EQ(network_->getGameRoomId(), GameRoomId{"server uuid"});
		EXPECT_TRUE(createGameRoomEventCalled);
		EXPECT_TRUE(network_->isInsideRoom());
	}

	TEST_F(NetworkTest, network_joinGameRoom) {
		// Given
		bool joinGameRoomEventCalled = false;
		auto connection = network_->joinGameRoomEvent.connect([&](const JoinGameRoomEvent& joinGameRoomEvent) {
			joinGameRoomEventCalled = true;
		});

		mockReceiveGameRoomJoined(GameRoomId{"server uuid"}, ClientId{"client uuid"});

		// When
		EXPECT_FALSE(joinGameRoomEventCalled);
		EXPECT_FALSE(network_->isInsideRoom());
		EXPECT_NE(network_->getGameRoomId(), GameRoomId{"server uuid"});
		pollOne();

		// Then.
		EXPECT_EQ(network_->getGameRoomId(), GameRoomId{"server uuid"});
		EXPECT_TRUE(joinGameRoomEventCalled);
		EXPECT_TRUE(network_->isInsideRoom());
	}

	TEST_F(NetworkTest, receiveGameLoobyContainingAllSlotTypes_thenEventsAreTriggered) {
		// Given
		mockReceiveGameRoomCreated(GameRoomId{"server uuid"}, ClientId{"client uuid 0"});
		pollOne();

		std::vector<PlayerSlotEvent> actualPlayerSlotEvents;
		auto connection = network_->playerSlotEvent.connect([&](const PlayerSlotEvent& playerSlotEvent) {
			actualPlayerSlotEvents.push_back(playerSlotEvent);
		});

		auto gameLooby = wrapperFromServer.mutable_game_looby();
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_REMOTE, ClientId{"client uuid 0"}, "name 0");
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_CLOSED_SLOT, ClientId{""}, "");
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_REMOTE, ClientId{"client uuid 2"}, "name 2", false);
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_OPEN_SLOT, ClientId{""}, "");
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_REMOTE, ClientId{"client uuid 4"}, "name 4", true);
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_REMOTE, ClientId{"client uuid 0"}, "name 5", true);

		expectCallClientReceive(wrapperFromServer);

		// When
		pollOne();

		// Then.
		EXPECT_EQ(network_->getGameRoomId(), GameRoomId{"server uuid"});
		EXPECT_EQ(actualPlayerSlotEvents.size(), 6);
		expectEqual(actualPlayerSlotEvents[0], 0, game::Human{
			.name = "name 0"
		});
		expectEqual(actualPlayerSlotEvents[1], 1, game::ClosedSlot{});
		expectEqual(actualPlayerSlotEvents[2], 2, game::Remote{
			.name = "name 2",
			.ai = false,
		});
		expectEqual(actualPlayerSlotEvents[3], 3, game::OpenSlot{});
		expectEqual(actualPlayerSlotEvents[4], 4, game::Remote{
			.name = "name 4",
			.ai = true,
		});
		expectEqual(actualPlayerSlotEvents[5], 5, game::Ai{
			.name = "name 5"
		});
	}

	TEST_F(NetworkTest, receiveGameLoobyAndSetSlot) {
		// Given
		mockReceiveGameRoomCreated(GameRoomId{"server uuid"}, ClientId{"client uuid 0"});
		pollOne();

		auto gameLooby = wrapperFromServer.mutable_game_looby();
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_OPEN_SLOT, ClientId{"client uuid 0"}, "name 0");

		expectCallClientReceive(wrapperFromServer);
		ProtobufMessage messageToServer;
		debugClientOnNetwork_->setSentToServerCallback([&](const ProtobufMessage& message) {
			messageToServer = message;
		});

		// When
		pollOne();
		network_->setPlayerSlot(game::Human{
			.name = "name 0"
		}, 0);

		// Then
		tp_c2s::Wrapper wrapperToServer;
		messageToServer.parseBodyInto(wrapperFromServer);
		auto playerSlot = wrapperToServer.player_slot();
		EXPECT_EQ(playerSlot.slot_type(), tp_c2s::PlayerSlot_SlotType_HUMAN);
		EXPECT_EQ(playerSlot.index(), 0);
		EXPECT_EQ(playerSlot.name(), "name 0");
	}

	TEST_F(NetworkTest, receiveGameLoobyAndSetSlotOutsideRange_thenIgnoreSlot) {
		// Given
		mockReceiveGameRoomCreated(GameRoomId{"server uuid"}, ClientId{"client uuid 0"});
		pollOne();

		auto gameLooby = wrapperFromServer.mutable_game_looby();
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_OPEN_SLOT, ClientId{"client uuid 0"}, "name 0");

		expectCallClientReceive(wrapperFromServer);
		pollOne();

		// When
		debugClientOnNetwork_->setSentToServerCallback([](const ProtobufMessage& message) {
			FAIL() << "send should not be called";
		});

		network_->setPlayerSlot(game::Human{
			.name = "name 1"
		}, 1);
	}

	TEST_F(NetworkTest, receiveGameLoobyWithMultipleClientsAndSetSlot) {
		// Given
		mockReceiveGameRoomCreated(GameRoomId{"server uuid"}, ClientId{"client uuid 0"});
		pollOne();

		auto gameLooby = wrapperFromServer.mutable_game_looby();
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_REMOTE, ClientId{"client uuid 0"}, "name 0");
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_REMOTE, ClientId{"client uuid 1"}, "name 1");
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_OPEN_SLOT, ClientId{""}, "");
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_OPEN_SLOT, ClientId{""}, "");

		expectCallClientReceive(wrapperFromServer);
		ProtobufMessage messageToServer;
		debugClientOnNetwork_->setSentToServerCallback([&](const ProtobufMessage& message) {
			messageToServer = message;
		});

		// When
		pollOne();
		network_->setPlayerSlot(game::Human{
			.name = "name 2"
		}, 2);

		// Then
		tp_c2s::Wrapper wrapperToServer;
		messageToServer.parseBodyInto(wrapperFromServer);
		auto playerSlot = wrapperToServer.player_slot();
		EXPECT_EQ(playerSlot.slot_type(), tp_c2s::PlayerSlot_SlotType_HUMAN);
		EXPECT_EQ(playerSlot.index(), 2);
		EXPECT_EQ(playerSlot.name(), "name 2");
	}

}
