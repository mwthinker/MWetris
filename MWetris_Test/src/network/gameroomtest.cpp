#include "testutil.h"

#include <network/gameroom.h>
#include <network/server.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>
#include <set>

using namespace ::testing;

namespace mwetris::network {

	class MockServer : public network::Server {
	public:
		MOCK_METHOD(void, sendToClient, (const std::string& clientUuid, const google::protobuf::MessageLite& message), (override));
		MOCK_METHOD(void, triggerConnectedClientEvent, (const ConnectedClient& connectedClient), (override));
		MOCK_METHOD(void, triggerPlayerSlotEvent, (const std::vector<Slot>& slots), (override));
		MOCK_METHOD(void, triggerInitGameEvent, (const game::InitGameEvent& initGameEvent), (override));
	};

	class GameRoomTest : public ::testing::Test {
	protected:
		GameRoomTest() {
		}

		~GameRoomTest() override {}

		void SetUp() override {
			gameRoom_ = std::make_shared<GameRoom>();
		}

		void TearDown() override {
			wrapperFromClient.Clear();
		}

		tp_c2s::Wrapper wrapperFromClient;
		tp_s2c::Wrapper wrapperToClient;
		NiceMock<MockServer> mockServer_;
		std::shared_ptr<GameRoom> gameRoom_;
	};

	void assertEqSlot(const tp_s2c::GameLooby_Slot& slot, tp_s2c::GameLooby_SlotType slotType, const std::string& name, bool ai, const std::string& clientUuid) {
		ASSERT_EQ(slot.slot_type(), slotType);
		ASSERT_EQ(slot.name(), name);
		ASSERT_EQ(slot.ai(), false);
		ASSERT_EQ(slot.client_uuid(), clientUuid);
	}

	void assertUniquePlayerUuids(const tp_s2c::GameLooby& playerSlot, int size) {
		std::set<std::string> playerUuids;
		for (const auto& slot : playerSlot.slots()) {
			playerUuids.insert(slot.player_uuid());
		}
		ASSERT_EQ(playerUuids.size(), size);
	}

	TEST_F(GameRoomTest, receiveJoinGameRoom) {
		// Given
		auto joinGameRoom = wrapperFromClient.mutable_join_game_room();
		joinGameRoom->set_server_uuid("server uuid");

		// When
		gameRoom_->receiveMessage(mockServer_, "client uuid 0", wrapperFromClient);

		// Then
		ASSERT_EQ(gameRoom_->getConnectedClientUuids().size(), 1);
		ASSERT_EQ(gameRoom_->getConnectedClientUuids()[0], "client uuid 0");
	}

	TEST_F(GameRoomTest, receiveGameRoomCreated) {
		// Given
		auto createGameRoom = wrapperFromClient.mutable_create_game_room();
		createGameRoom->set_name("name");

		// When
		gameRoom_->receiveMessage(mockServer_, "client uuid 0", wrapperFromClient);

		// Then
		ASSERT_EQ(gameRoom_->getConnectedClientUuids().size(), 1);
		ASSERT_EQ(gameRoom_->getConnectedClientUuids()[0], "client uuid 0");
	}

	TEST_F(GameRoomTest, receivePlayerSlot_thenSendGameLooby) {
		// Given
		auto createGameRoom = wrapperFromClient.mutable_create_game_room();
		createGameRoom->set_name("name");
		gameRoom_->receiveMessage(mockServer_, "client uuid 0", wrapperFromClient);
		wrapperFromClient.Clear();

		auto mutablePlayerSlot = wrapperFromClient.mutable_player_slot();
		mutablePlayerSlot->set_index(0);
		mutablePlayerSlot->set_name("name 0");
		mutablePlayerSlot->set_slot_type(tp_c2s::PlayerSlot_SlotType_HUMAN);
		
		tp_s2c::Wrapper wrapperToClient;
		std::string clientUuid;
		EXPECT_CALL(mockServer_, sendToClient(_, _)).WillOnce(
			Invoke([&](const std::string& uuid, const google::protobuf::MessageLite& message) {
				clientUuid = uuid;
				wrapperToClient = dynamic_cast<const tp_s2c::Wrapper&>(message);
			}));

		// When
		gameRoom_->receiveMessage(mockServer_, "client uuid 0", wrapperFromClient);

		// Then
		ASSERT_EQ(clientUuid, "client uuid 0");
		const auto& playerSlot = wrapperToClient.game_looby();
		
		assertUniquePlayerUuids(playerSlot, 2);
		ASSERT_EQ(playerSlot.slots().size(), 4);
		assertEqSlot(playerSlot.slots(0), tp_s2c::GameLooby_SlotType_REMOTE, "name 0", false, "client uuid 0");
		assertEqSlot(playerSlot.slots(1), tp_s2c::GameLooby_SlotType_OPEN_SLOT, "", false, "");
		assertEqSlot(playerSlot.slots(2), tp_s2c::GameLooby_SlotType_OPEN_SLOT, "", false, "");
		assertEqSlot(playerSlot.slots(3), tp_s2c::GameLooby_SlotType_OPEN_SLOT, "", false, "");
	}

}
