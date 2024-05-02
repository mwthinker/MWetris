#include "testutil.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <network/client.h>
#include <network/network.h>

#include <shared.pb.h>
#include <client_to_server.pb.h>
#include <server_to_client.pb.h>

using namespace ::testing;

namespace mwetris::network {

	class MockClient : public network::Client {
	public:
		MOCK_METHOD(bool, receive, (ProtobufMessage& message), (override));
		MOCK_METHOD(void, send, (ProtobufMessage&& message), (override));
		MOCK_METHOD(void, acquire, (ProtobufMessage& message), (override));
		MOCK_METHOD(void, release, (ProtobufMessage&& message), (override));
	};

	class NetworkTest : public ::testing::Test {
	protected:

		NetworkTest() {
			mockClient_ = std::make_shared<MockClient>();
		}

		~NetworkTest() override {}

		void SetUp() override {
			network_ = std::make_shared<Network>(mockClient_);
		}

		void TearDown() override {
			wrapperFromServer.Clear();
		}

		void mockReceiveGameRoomCreated(const std::string& serverUuid, const std::string& clientUuid) {
			tp_s2c::Wrapper wrapperFromServer;
			auto gameRoomCreated = wrapperFromServer.mutable_game_room_created();
			gameRoomCreated->set_client_uuid(clientUuid);
			gameRoomCreated->set_server_uuid(serverUuid);

			expectCallClientReceive(wrapperFromServer);
			wrapperFromServer.Clear();
		}

		void expectCallClientReceive(const tp_s2c::Wrapper& wrapper) {
			auto message = createMessage(wrapper);
			EXPECT_CALL(*mockClient_, receive(_))
				.WillOnce(DoAll(SetArgReferee<0>(message), Return(true)))
				.WillOnce(Return(false));
		}

		tp_s2c::Wrapper wrapperFromServer;
		std::shared_ptr<MockClient> mockClient_;
		std::shared_ptr<Network> network_;
	};

	TEST_F(NetworkTest, network_isNotInsideRoom) {
		ASSERT_FALSE(network_->isInsideRoom());
		ASSERT_EQ(network_->getGameRoomUuid(), "");
	}

	TEST_F(NetworkTest, network_createGameRoom) {
		// Given
		bool createGameRoomEventCalled = false;
		auto connection = network_->createGameRoomEvent.connect([&](const CreateGameRoomEvent& createGameRoomEvent) {
			createGameRoomEventCalled = true;
		});
		
		mockReceiveGameRoomCreated("server uuid", "client uuid");

		// When
		EXPECT_FALSE(createGameRoomEventCalled);
		EXPECT_NE(network_->getGameRoomUuid(), "server uuid");
		network_->update();

		// Then.
		EXPECT_EQ(network_->getGameRoomUuid(), "server uuid");
		EXPECT_TRUE(createGameRoomEventCalled);
	}

	TEST_F(NetworkTest, receiveGameLoobyContainingAllSlotTypes_thenEventsAreTriggered) {
		// Given
		mockReceiveGameRoomCreated("server uuid", "client uuid 0");
		network_->update();

		std::vector<PlayerSlotEvent> actualPlayerSlotEvents;
		auto connection = network_->playerSlotEvent.connect([&](const PlayerSlotEvent& playerSlotEvent) {
			actualPlayerSlotEvents.push_back(playerSlotEvent);
		});
		
		auto gameLooby = wrapperFromServer.mutable_game_looby();
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_REMOTE, "client uuid 0", "name 0");
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_CLOSED_SLOT, "", "");
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_REMOTE, "client uuid 2", "name 2", false);
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_OPEN_SLOT, "", "");
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_REMOTE, "client uuid 4", "name 4", true);
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_REMOTE, "client uuid 0", "name 5", true);

		expectCallClientReceive(wrapperFromServer);

		// When
		network_->update();

		// Then.
		EXPECT_EQ(network_->getGameRoomUuid(), "server uuid");
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
		mockReceiveGameRoomCreated("server uuid", "client uuid 0");
		network_->update();

		auto gameLooby = wrapperFromServer.mutable_game_looby();
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_OPEN_SLOT, "client uuid 0", "name 0");

		expectCallClientReceive(wrapperFromServer);
		ProtobufMessage messageToClient;
		EXPECT_CALL(*mockClient_, send(_))
			.WillOnce(SaveArg<0>(&messageToClient));

		// When
		network_->update();
		network_->setPlayerSlot(game::Human{
			.name = "name 0"
		}, 0);

		// Then
		tp_c2s::Wrapper wrapperToServer;
		wrapperToServer.ParseFromArray(messageToClient.getBodyData(), messageToClient.getBodySize());
		auto playerSlot = wrapperToServer.player_slot();
		EXPECT_EQ(playerSlot.slot_type(), tp_c2s::PlayerSlot_SlotType_HUMAN);
		EXPECT_EQ(playerSlot.index(), 0);
		EXPECT_EQ(playerSlot.name(), "name 0");
	}

	TEST_F(NetworkTest, receiveGameLoobyAndSetSlotOutsideRange_thenIgnoreSlot) {
		// Given
		mockReceiveGameRoomCreated("server uuid", "client uuid 0");
		network_->update();

		auto gameLooby = wrapperFromServer.mutable_game_looby();
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_OPEN_SLOT, "client uuid 0", "name 0");

		expectCallClientReceive(wrapperFromServer);
		network_->update();

		// When
		EXPECT_CALL(*mockClient_, send(_)).WillOnce(Invoke([](const ProtobufMessage& message) {
			FAIL() << "send should not be called";
		}));

		network_->setPlayerSlot(game::Human{
			.name = "name 1"
			}, 1);
	}

}
