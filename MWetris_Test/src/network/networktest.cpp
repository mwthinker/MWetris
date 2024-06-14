#include "testutil.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <mwetris/network/client.h>
#include <mwetris/network/network.h>
#include <mwetris/network/id.h>
#include <mwetris/network/debugclient.h>
#include <mwetris/network/debugserver.h>

#include <shared.pb.h>
#include <client_to_server.pb.h>
#include <server_to_client.pb.h>
#include <asio.hpp>

using namespace ::testing;

namespace mwetris::network {

	namespace {

		ProtobufMessage createMessage(const google::protobuf::MessageLite& wrapper) {
			ProtobufMessage message;
			message.clear();
			message.setBuffer(wrapper);
			return message;
		}

		asio::io_context ioContext_;
		std::queue<ProtobufMessage> receivedMessages_;
		asio::high_resolution_timer timer_{ioContext_};
		bool shutdown = false;

		asio::awaitable<ProtobufMessage> receive() {
			while (receivedMessages_.empty() && !shutdown) {
				co_await timer_.async_wait(asio::use_awaitable);
				timer_.expires_after(std::chrono::seconds{0});
			}
			auto message = std::move(receivedMessages_.front());
			receivedMessages_.pop();
			co_return message;
		}

	}

	class MockClient : public network::Client {
	public:
		asio::awaitable<ProtobufMessage> receive() override {
			while (receivedMessages_.empty()) {
				if (shutdown) {
					co_return ProtobufMessage{};
				}

				co_await timer_.async_wait(asio::use_awaitable);
				timer_.expires_after(std::chrono::seconds{0});
			}
			auto message = std::move(receivedMessages_.front());
			receivedMessages_.pop();
			co_return message;
		}

		MOCK_METHOD(void, send, (ProtobufMessage&& message), (override));
		MOCK_METHOD(void, acquire, (ProtobufMessage& message), (override));
		MOCK_METHOD(void, release, (ProtobufMessage&& message), (override));
		MOCK_METHOD(asio::io_context&, getIoContext, (), (override));
		MOCK_METHOD(void, stop, (), (override));
	};

	class NetworkTest : public ::testing::Test {
	protected:
		NetworkTest() {
			mockClient_ = std::make_shared<NiceMock<MockClient>>();

			ON_CALL(*mockClient_, getIoContext())
				.WillByDefault(ReturnRef(ioContext_));
			ON_CALL(*mockClient_, acquire(_))
				.WillByDefault(SetArgReferee<0>(ProtobufMessage{}));
		}

		~NetworkTest() override {
		}

		void SetUp() override {
			timer_.expires_after(std::chrono::seconds{0});

			shutdown = false;
			receivedMessages_ = std::queue<ProtobufMessage>();
			network_ = std::make_shared<Network>(mockClient_);
			shutdown = false;
		}

		void TearDown() override {
			mockClient_ = nullptr;
			shutdown = true;
			network_->stop();
			ioContext_.poll();

			network_ = nullptr;
			wrapperFromServer.Clear();
			ioContext_.restart();
		}

		void pollOne() {
			ioContext_.poll_one();
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
			receivedMessages_.push(createMessage(wrapper));
		}

		std::shared_ptr<NiceMock<MockClient>> mockClient_;
		tp_s2c::Wrapper wrapperFromServer;
		std::shared_ptr<Network> network_;
	};

	TEST_F(NetworkTest, network_isNotInsideRoom) {
		ASSERT_FALSE(network_->isInsideRoom());
		ASSERT_EQ(network_->getGameRoomId(), GameRoomId{});
	}

	TEST_F(NetworkTest, network_createGameRoom) {
		// Given
		bool createGameRoomEventCalled = false;
		mw::signals::ScopedConnection connection = network_->createGameRoomEvent.connect([&](const CreateGameRoomEvent& createGameRoomEvent) {
			createGameRoomEventCalled = true;
		});

		mockReceiveGameRoomCreated(GameRoomId{"server uuid"}, ClientId{"client uuid"});

		// When
		EXPECT_FALSE(network_->isInsideRoom());
		EXPECT_FALSE(createGameRoomEventCalled);
		std::string expected = network_->getGameRoomId().c_str();
		EXPECT_NE(network_->getGameRoomId(), GameRoomId{"server uuid"});
		pollOne();

		// Then.
		std::string result = network_->getGameRoomId().c_str();
		EXPECT_EQ(network_->getGameRoomId(), GameRoomId{"server uuid"});
		EXPECT_TRUE(createGameRoomEventCalled);
		EXPECT_TRUE(network_->isInsideRoom());
	}

	TEST_F(NetworkTest, network_joinGameRoom) {
		// Given
		bool joinGameRoomEventCalled = false;
		mw::signals::ScopedConnection connection = network_->joinGameRoomEvent.connect([&](const JoinGameRoomEvent& joinGameRoomEvent) {
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
		mw::signals::ScopedConnection connection = network_->playerSlotEvent.connect([&](const PlayerSlotEvent& playerSlotEvent) {
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

		wrapperFromServer.Clear();
		auto gameLooby = wrapperFromServer.mutable_game_looby();
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_OPEN_SLOT, ClientId{}, "");
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_OPEN_SLOT, ClientId{}, "");

		expectCallClientReceive(wrapperFromServer);
		ProtobufMessage messageToServer;
		EXPECT_CALL(*mockClient_, send(_))
			.WillOnce(SaveArg<0>(&messageToServer));

		// When
		pollOne();
		network_->setPlayerSlot(game::Human{
			.name = "name 0"
		}, 1);

		// Then
		tp_c2s::Wrapper wrapperToServer;
		messageToServer.parseBodyInto(wrapperToServer);

		const auto& playerSlot = wrapperToServer.player_slot();
		auto name = playerSlot.name();
		EXPECT_EQ(playerSlot.slot_type(), tp_c2s::PlayerSlot_SlotType_HUMAN);
		EXPECT_EQ(playerSlot.index(), 1);
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
		ON_CALL(*mockClient_, send(_)).WillByDefault(Invoke([](const ProtobufMessage& message) {
			FAIL() << "send should not be called";
		}));

		network_->setPlayerSlot(game::Human{
			.name = "name 1"
		}, 1);
	}

	TEST_F(NetworkTest, receiveGameLoobyWithMultipleClientsAndSetSlot) {
		// Given
		mockReceiveGameRoomCreated(GameRoomId{"server uuid"}, ClientId{"client uuid 0"});
		pollOne();

		wrapperFromServer.Clear();
		auto gameLooby = wrapperFromServer.mutable_game_looby();
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_REMOTE, ClientId{"client uuid 0"}, "name 0");
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_REMOTE, ClientId{"client uuid 1"}, "name 1");
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_OPEN_SLOT, ClientId{""}, "");
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_OPEN_SLOT, ClientId{""}, "");

		expectCallClientReceive(wrapperFromServer);
		ProtobufMessage messageToServer;
		EXPECT_CALL(*mockClient_, send(_))
			.WillOnce(SaveArg<0>(&messageToServer));

		// When
		pollOne();
		network_->setPlayerSlot(game::Human{
			.name = "name 2"
		}, 2);

		// Then
		tp_c2s::Wrapper wrapperToServer;
		messageToServer.parseBodyInto(wrapperToServer);

		const auto& playerSlot = wrapperToServer.player_slot();
		auto name = playerSlot.name();
		EXPECT_EQ(playerSlot.slot_type(), tp_c2s::PlayerSlot_SlotType_HUMAN);
		EXPECT_EQ(playerSlot.index(), 2);
		EXPECT_EQ(playerSlot.name(), "name 2");
	}

}
