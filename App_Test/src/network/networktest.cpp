#include "testutil.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <mwetris/cnetwork/network.h>

#include <network/client.h>
#include <network/debugclient.h>
#include <network/debugserver.h>
#include <network/id.h>

#include <protocol/shared.pb.h>
#include <protocol/client_to_server.pb.h>
#include <protocol/server_to_client.pb.h>

#include <asio.hpp>

using namespace ::testing;

namespace mwetris::cnetwork {

	namespace {

		network::ProtobufMessage createMessage(const google::protobuf::MessageLite& wrapper) {
			network::ProtobufMessage message;
			message.clear();
			message.setBuffer(wrapper);
			return message;
		}

		asio::io_context ioContext_;
		std::queue<network::ProtobufMessage> receivedMessages_;
		asio::high_resolution_timer timer_{ioContext_};
		bool shutdown = false;

		asio::awaitable<network::ProtobufMessage> receive() {
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
		asio::awaitable<network::ProtobufMessage> receive() override {
			while (receivedMessages_.empty()) {
				if (shutdown) {
					co_return network::ProtobufMessage{};
				}

				co_await timer_.async_wait(asio::use_awaitable);
				timer_.expires_after(std::chrono::seconds{0});
			}
			auto message = std::move(receivedMessages_.front());
			receivedMessages_.pop();
			co_return message;
		}

		MOCK_METHOD(void, send, (network::ProtobufMessage&& message), (override));
		MOCK_METHOD(void, acquire, (network::ProtobufMessage& message), (override));
		MOCK_METHOD(void, release, (network::ProtobufMessage&& message), (override));
		MOCK_METHOD(asio::io_context&, getIoContext, (), (override));
		MOCK_METHOD(void, stop, (), (override));
		MOCK_METHOD(bool, isConnected, (), (const, override));
		MOCK_METHOD(void, reconnect, (), (override));
	};

	class NetworkTest : public ::testing::Test {
	protected:
		NetworkTest() {
			mockClient_ = std::make_shared<NiceMock<MockClient>>();

			ON_CALL(*mockClient_, getIoContext())
				.WillByDefault(ReturnRef(ioContext_));
			ON_CALL(*mockClient_, acquire(_))
				.WillByDefault(SetArgReferee<0>(network::ProtobufMessage{}));
		}

		~NetworkTest() override {
		}

		void SetUp() override {
			timer_.expires_after(std::chrono::seconds{0});

			shutdown = false;
			receivedMessages_ = std::queue<network::ProtobufMessage>();
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

		void mockReceiveGameRoomJoined(const network::GameRoomId& gameRoomId, const network::ClientId& clientId) {
			tp_s2c::Wrapper wrapperFromServer;
			auto gameRoomJoined = wrapperFromServer.mutable_game_room_joined();
			fromCppToProto(clientId, *gameRoomJoined->mutable_client_id());
			fromCppToProto(gameRoomId, *gameRoomJoined->mutable_game_room_id());

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
		ASSERT_EQ(network_->getGameRoomId(), network::GameRoomId{});
	}

	TEST_F(NetworkTest, network_createGameRoom) {
		// Given
		bool createGameRoomEventCalled = false;
		mw::signals::ScopedConnection connection = network_->networkEvent.connect([&](const NetworkEvent& networkEvent) {
			if (auto createGameRoomEvent = std::get_if<JoinGameRoomEvent>(&networkEvent)) {
				createGameRoomEventCalled = true;
			}
		});

		mockReceiveGameRoomJoined(network::GameRoomId{"server id"}, network::ClientId{"client id"});

		// When
		EXPECT_FALSE(network_->isInsideRoom());
		EXPECT_FALSE(createGameRoomEventCalled);
		std::string expected = network_->getGameRoomId().c_str();
		EXPECT_NE(network_->getGameRoomId(), network::GameRoomId{"server id"});
		pollOne();

		// Then.
		std::string result = network_->getGameRoomId().c_str();
		EXPECT_EQ(network_->getGameRoomId(), network::GameRoomId{"server id"});
		EXPECT_TRUE(createGameRoomEventCalled);
		EXPECT_TRUE(network_->isInsideRoom());
	}

	TEST_F(NetworkTest, network_joinGameRoom) {
		// Given
		bool joinGameRoomEventCalled = false;
		mw::signals::ScopedConnection connection = network_->networkEvent.connect([&](const NetworkEvent& networkEvent) {
			if (auto joinGameRoomEvent = std::get_if<JoinGameRoomEvent>(&networkEvent)) {
				joinGameRoomEventCalled = true;
			}
		});

		mockReceiveGameRoomJoined(network::GameRoomId{"server id"}, network::ClientId{"client id"});

		// When
		EXPECT_FALSE(joinGameRoomEventCalled);
		EXPECT_FALSE(network_->isInsideRoom());
		EXPECT_NE(network_->getGameRoomId(), network::GameRoomId{"server id"});
		pollOne();

		// Then.
		EXPECT_EQ(network_->getGameRoomId(), network::GameRoomId{"server id"});
		EXPECT_TRUE(joinGameRoomEventCalled);
		EXPECT_TRUE(network_->isInsideRoom());
	}

	TEST_F(NetworkTest, receiveGameLoobyContainingAllSlotTypes_thenEventsAreTriggered) {
		// Given
		mockReceiveGameRoomJoined(network::GameRoomId{"server id"}, network::ClientId{"client id 0"});
		pollOne();

		std::vector<PlayerSlotEvent> actualPlayerSlotEvents;
		mw::signals::ScopedConnection connection = network_->networkEvent.connect([&](const NetworkEvent& networkEvent) {
			if (auto playerSlotEvent = std::get_if<PlayerSlotEvent>(&networkEvent)) {
				actualPlayerSlotEvents.push_back(*playerSlotEvent);
			}
		});

		auto gameLooby = wrapperFromServer.mutable_game_looby();
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_REMOTE, network::ClientId{"client id 0"}, "name 0");
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_CLOSED_SLOT, network::ClientId{""}, "");
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_REMOTE, network::ClientId{"client id 2"}, "name 2", false);
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_OPEN_SLOT, network::ClientId{""}, "");
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_REMOTE, network::ClientId{"client id 4"}, "name 4", true);
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_REMOTE, network::ClientId{"client id 0"}, "name 5", true);

		expectCallClientReceive(wrapperFromServer);

		// When
		pollOne();

		// Then.
		EXPECT_EQ(network_->getGameRoomId(), network::GameRoomId{"server id"});
		EXPECT_EQ(actualPlayerSlotEvents.size(), 6);
		network::expectEqual(actualPlayerSlotEvents[0], 0, game::Human{
			.name = "name 0"
		});
		network::expectEqual(actualPlayerSlotEvents[1], 1, game::ClosedSlot{});
		network::expectEqual(actualPlayerSlotEvents[2], 2, game::Remote{
			.name = "name 2",
			.ai = false,
		});
		network::expectEqual(actualPlayerSlotEvents[3], 3, game::OpenSlot{});
		network::expectEqual(actualPlayerSlotEvents[4], 4, game::Remote{
			.name = "name 4",
			.ai = true,
		});
		network::expectEqual(actualPlayerSlotEvents[5], 5, game::Ai{
			.name = "name 5"
		});
	}

	TEST_F(NetworkTest, receiveGameLoobyAndSetSlot) {
		// Given
		mockReceiveGameRoomJoined(network::GameRoomId{"server id"}, network::ClientId{"client id 0"});
		pollOne();

		wrapperFromServer.Clear();
		auto gameLooby = wrapperFromServer.mutable_game_looby();
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_OPEN_SLOT, network::ClientId{}, "");
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_OPEN_SLOT, network::ClientId{}, "");

		expectCallClientReceive(wrapperFromServer);
		network::ProtobufMessage messageToServer;
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
		EXPECT_EQ(playerSlot.slot_type(), tp_c2s::PlayerSlot_SlotType_HUMAN);
		EXPECT_EQ(playerSlot.index(), 1);
		EXPECT_EQ(playerSlot.name(), "name 0");
	}

	TEST_F(NetworkTest, receiveGameLoobyAndSetSlotOutsideRange_thenIgnoreSlot) {
		// Given
		mockReceiveGameRoomJoined(network::GameRoomId{"server id"}, network::ClientId{"client id 0"});
		pollOne();

		auto gameLooby = wrapperFromServer.mutable_game_looby();
		network::addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_OPEN_SLOT, network::ClientId{"client id 0"}, "name 0");

		expectCallClientReceive(wrapperFromServer);
		pollOne();

		// When
		ON_CALL(*mockClient_, send(_)).WillByDefault(Invoke([](const network::ProtobufMessage& message) {
			FAIL() << "send should not be called";
		}));

		network_->setPlayerSlot(game::Human{
			.name = "name 1"
		}, 1);
	}

	TEST_F(NetworkTest, receiveGameLoobyWithMultipleClientsAndSetSlot) {
		// Given
		mockReceiveGameRoomJoined(network::GameRoomId{"server id"}, network::ClientId{"client id 0"});
		pollOne();

		wrapperFromServer.Clear();
		auto gameLooby = wrapperFromServer.mutable_game_looby();
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_REMOTE, network::ClientId{"client id 0"}, "name 0");
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_REMOTE, network::ClientId{"client id 1"}, "name 1");
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_OPEN_SLOT, network::ClientId{""}, "");
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_OPEN_SLOT, network::ClientId{""}, "");

		expectCallClientReceive(wrapperFromServer);
		network::ProtobufMessage messageToServer;
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
		EXPECT_EQ(playerSlot.slot_type(), tp_c2s::PlayerSlot_SlotType_HUMAN);
		EXPECT_EQ(playerSlot.index(), 2);
		EXPECT_EQ(playerSlot.name(), "name 2");
	}

	void addCreateGamePlayer(tp_s2c::CreateGame& createGame, const network::ClientId& clientId, const std::string& name, bool ai) {
		auto player = createGame.add_players();
		player->set_ai(false);
		player->set_name("name 0");
		player->set_current(tp::BlockType::J);
		player->set_next(tp::BlockType::L);
		player->set_points(2);
		fromCppToProto(network::ClientId{"client id 0"}, *player->mutable_client_id());
	}

	// Assumes that the following events are received in order:
	// - GameRoomCreated
	// - GameLooby
	// - CreateGame
	// TODO! Maybe CreateGame does not need to contain the same players as GameLooby??
	TEST_F(NetworkTest, receiveCreateGame) {
		// Given
		mockReceiveGameRoomJoined(network::GameRoomId{"server id"}, network::ClientId{"client id 0"});
		pollOne();

		wrapperFromServer.Clear();
		auto gameLooby = wrapperFromServer.mutable_game_looby();
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_REMOTE, network::ClientId{"client id 0"}, "name 0");
		addPlayerSlot(*gameLooby, tp_s2c::GameLooby_SlotType_REMOTE, network::ClientId{"client id 1"}, "name 1");
		expectCallClientReceive(wrapperFromServer);
		pollOne();

		wrapperFromServer.Clear();
		auto createGame = wrapperFromServer.mutable_create_game();
		createGame->set_width(10);
		createGame->set_height(20);
		addCreateGamePlayer(*createGame, network::ClientId{"client id 0"}, "name 0", true);
		addCreateGamePlayer(*createGame, network::ClientId{"client id 1"}, "name 1", true);
		expectCallClientReceive(wrapperFromServer);
		wrapperFromServer.Clear();

		CreateGameEvent actualCreateGameEvent;
		mw::signals::ScopedConnection connection = network_->networkEvent.connect([&](const NetworkEvent& networkEvent) {
			if (auto createGameEvent = std::get_if<CreateGameEvent>(&networkEvent)) {
				actualCreateGameEvent = *createGameEvent;
			}
		});

		// When
		pollOne();

		// Then
		EXPECT_EQ(actualCreateGameEvent.players.size(), 2);
		if (actualCreateGameEvent.players.size() == 2) {
			EXPECT_EQ(actualCreateGameEvent.players[0]->isLocal(), true);
			EXPECT_EQ(actualCreateGameEvent.players[1]->isRemote(), true);
		} else {
			FAIL() << "actualCreateGameEvent.players.size() is not 2";
		}
	}

}
