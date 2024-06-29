#include "testutil.h"

#include <mwetris/network/gameroom.h>
#include <mwetris/network/server.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>
#include <set>

using namespace ::testing;

namespace mwetris::network {

	namespace {

		void assertEqSlot(const tp_s2c::GameLooby& gameLooby, int index, tp_s2c::GameLooby_SlotType slotType, const std::string& name, bool ai, const ClientId& clientId) {
			if (index >= gameLooby.slots().size()) {
				FAIL() << "index out of range, index = " << index;
			}
			auto slot = gameLooby.slots(index);
			ASSERT_EQ(slot.slot_type(), slotType) << "Error for slot index = " << index;
			ASSERT_EQ(slot.name(), name) << "Error for slot index = " << index;
			ASSERT_EQ(slot.ai(), false) << "Error for slot index = " << index;
			ASSERT_EQ(slot.client_id(), clientId) << "Error for slot index = " << index;
		}

	}

	class MockServer : public network::Server {
	public:
		MOCK_METHOD(void, sendToClient, (const ClientId& clientId, const google::protobuf::MessageLite& message), (override));
		MOCK_METHOD(void, triggerConnectedClientEvent, (const ConnectedClient& connectedClient), (override));
		MOCK_METHOD(void, triggerPlayerSlotEvent, (const std::vector<Slot>& slots), (override));
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

	void assertUniquePlayerIds(const tp_s2c::GameLooby& playerSlot, int size) {
		std::set<PlayerId> playerIds;
		for (const auto& slot : playerSlot.slots()) {
			playerIds.insert(slot.player_id());
		}
		ASSERT_EQ(playerIds.size(), size);
	}

	TEST_F(GameRoomTest, receiveJoinGameRoom) {
		// Given
		auto joinGameRoom = wrapperFromClient.mutable_join_game_room();
		fromCppToProto(GameRoomId{"game room id"}, *joinGameRoom->mutable_game_room_id());

		// When
		gameRoom_->receiveMessage(mockServer_, ClientId{"client uuid 0"}, wrapperFromClient);

		// Then
		ASSERT_EQ(gameRoom_->getConnectedClientIds().size(), 1);
		ASSERT_EQ(gameRoom_->getConnectedClientIds()[0], ClientId{"client uuid 0"});
	}

	TEST_F(GameRoomTest, receiveGameRoomCreated) {
		// Given
		auto createGameRoom = wrapperFromClient.mutable_create_game_room();
		createGameRoom->set_name("name");

		// When
		gameRoom_->receiveMessage(mockServer_, ClientId{"client uuid 0"}, wrapperFromClient);

		// Then
		ASSERT_EQ(gameRoom_->getConnectedClientIds().size(), 1);
		ASSERT_EQ(gameRoom_->getConnectedClientIds()[0], ClientId{"client uuid 0"});
	}

	TEST_F(GameRoomTest, receivePlayerSlot_thenSendGameLooby) {
		// Given
		auto createGameRoom = wrapperFromClient.mutable_create_game_room();
		createGameRoom->set_name("name");
		gameRoom_->receiveMessage(mockServer_, ClientId{"client uuid 0"}, wrapperFromClient);
		wrapperFromClient.Clear();

		auto mutablePlayerSlot = wrapperFromClient.mutable_player_slot();
		mutablePlayerSlot->set_index(0);
		mutablePlayerSlot->set_name("name 0");
		mutablePlayerSlot->set_slot_type(tp_c2s::PlayerSlot_SlotType_HUMAN);
		
		tp_s2c::Wrapper wrapperToClient;
		ClientId clientId;
		EXPECT_CALL(mockServer_, sendToClient(_, _)).WillOnce(
			Invoke([&](const ClientId& id, const google::protobuf::MessageLite& message) {
				clientId = id;
				wrapperToClient = dynamic_cast<const tp_s2c::Wrapper&>(message);
			}));

		// When
		gameRoom_->receiveMessage(mockServer_, ClientId{"client uuid 0"}, wrapperFromClient);

		// Then
		ASSERT_EQ(clientId, ClientId{"client uuid 0"});
		const auto& playerSlot = wrapperToClient.game_looby();
		
		assertUniquePlayerIds(playerSlot, 2);
		ASSERT_EQ(playerSlot.slots().size(), 4);
		assertEqSlot(playerSlot, 0, tp_s2c::GameLooby_SlotType_REMOTE, "name 0", false, ClientId{"client uuid 0"});
		assertEqSlot(playerSlot, 1, tp_s2c::GameLooby_SlotType_OPEN_SLOT, "", false, ClientId{""});
		assertEqSlot(playerSlot, 2, tp_s2c::GameLooby_SlotType_OPEN_SLOT, "", false, ClientId{""});
		assertEqSlot(playerSlot, 3, tp_s2c::GameLooby_SlotType_OPEN_SLOT, "", false, ClientId{""});
	}

	void addPlayerSlotFromClient(tp_c2s::PlayerSlot& playerSlot, const std::string& clientUuid, int index, const std::string& name, tp_c2s::PlayerSlot_SlotType slotType) {

	}

	TEST_F(GameRoomTest, receivePlayerSlotFromDifferentClients_thenSendGameLooby) {
		// Given
		auto createGameRoom = wrapperFromClient.mutable_create_game_room();
		createGameRoom->set_name("name");
		gameRoom_->receiveMessage(mockServer_, ClientId{"client uuid 0"}, wrapperFromClient);
		wrapperFromClient.Clear();

		tp_s2c::Wrapper wrapperToClient;
		ClientId clientId;
		ON_CALL(mockServer_, sendToClient(_, _)).WillByDefault(
			Invoke([&](const ClientId& id, const google::protobuf::MessageLite& message) {
			clientId = id;
			wrapperToClient = dynamic_cast<const tp_s2c::Wrapper&>(message);
		}));

		// When
		auto mutablePlayerSlot = wrapperFromClient.mutable_player_slot();
		mutablePlayerSlot->set_index(0);
		mutablePlayerSlot->set_name("name 0");
		mutablePlayerSlot->set_slot_type(tp_c2s::PlayerSlot_SlotType_HUMAN);
		gameRoom_->receiveMessage(mockServer_, ClientId{"client uuid 0"}, wrapperFromClient);

		mutablePlayerSlot->set_index(1);
		mutablePlayerSlot->set_name("name 1");
		mutablePlayerSlot->set_slot_type(tp_c2s::PlayerSlot_SlotType_HUMAN);
		gameRoom_->receiveMessage(mockServer_, ClientId{"client uuid 1"}, wrapperFromClient);

		// Then
		ASSERT_EQ(clientId, ClientId{"client uuid 0"});
		const auto& playerSlot = wrapperToClient.game_looby();

		assertUniquePlayerIds(playerSlot, 3);
		ASSERT_EQ(playerSlot.slots().size(), 4);
		assertEqSlot(playerSlot, 0, tp_s2c::GameLooby_SlotType_REMOTE, "name 0", false, ClientId{"client uuid 0"});
		assertEqSlot(playerSlot, 1, tp_s2c::GameLooby_SlotType_REMOTE, "name 1", false, ClientId{"client uuid 1"});
		assertEqSlot(playerSlot, 2, tp_s2c::GameLooby_SlotType_OPEN_SLOT, "", false, ClientId{""});
		assertEqSlot(playerSlot, 3, tp_s2c::GameLooby_SlotType_OPEN_SLOT, "", false, ClientId{""});
	}

	TEST_F(GameRoomTest, receiveRemoveClient_thenSendRemoveClient) {
	}

}
