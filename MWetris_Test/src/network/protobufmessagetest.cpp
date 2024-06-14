#include "testutil.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <mwetris/network/protobufmessage.h>

#include <shared.pb.h>
#include <client_to_server.pb.h>
#include <server_to_client.pb.h>

#include <algorithm>

using namespace ::testing;

namespace mwetris::network {

	class ProtobufMessageTest : public ::testing::Test {
	protected:
		ProtobufMessageTest() {
		}

		~ProtobufMessageTest() override {
		}

		void SetUp() override {
		}

		void TearDown() override {
		}
	};

	TEST_F(ProtobufMessageTest, createMessage) {
		// Given
		ProtobufMessage message;
		EXPECT_EQ(message.getHeaderSize(), 2);
		EXPECT_EQ(message.getBodySize(), 0);
		EXPECT_EQ(message.getSize(), 0);

		// When
		tp::GameRoomId expected;
		expected.set_id("test");
		message.setBuffer(expected);

		// Then
		EXPECT_EQ(message.getHeaderSize(), 2);
		EXPECT_EQ(expected.ByteSizeLong(), message.getBodySize());
		EXPECT_EQ(message.getSize(), message.getHeaderSize() + message.getBodySize());
	}

	TEST_F(ProtobufMessageTest, clearMessage) {
		// Given
		ProtobufMessage message;
		tp::GameRoomId expected;
		expected.set_id("test");
		message.setBuffer(expected);

		// When
		EXPECT_EQ(message.getHeaderSize(), 2);
		EXPECT_GT(message.getBodySize(), 0);
		EXPECT_EQ(message.getSize(), message.getHeaderSize() + message.getBodySize());
		message.clear();

		// Then
		EXPECT_EQ(message.getHeaderSize(), 2);
		EXPECT_EQ(message.getBodySize(), 0);
		EXPECT_EQ(message.getSize(), 0);
	}

	TEST_F(ProtobufMessageTest, fromProtobufToMessageToProbuf) {
		// Given
		ProtobufMessage message;
		tp::GameRoomId expected;
		expected.set_id("test");
		message.setBuffer(expected);

		// When
		tp::GameRoomId result;
		message.parseBodyInto(result);

		// Then
		auto text = expected.id();
		EXPECT_EQ(expected.id(), std::string{"test"});
		EXPECT_EQ(expected.id(), result.id());
	}

	TEST_F(ProtobufMessageTest, insertDataUsingMutableBuffer) {
		// Given
		tp::GameRoomId package;
		package.set_id("test");
		ProtobufMessage message;
		
		// When
		message.reserveBodySize(static_cast<int>(package.ByteSizeLong()));
		auto buffer = message.getMutableBodyBuffer();
		package.SerializeToArray(buffer.data(), static_cast<int>(buffer.size()));

		tp::GameRoomId result;
		message.parseBodyInto(result);

		// Then
		EXPECT_EQ(package.id(), std::string{"test"});
		EXPECT_EQ(package.id(), result.id());
	}

}
