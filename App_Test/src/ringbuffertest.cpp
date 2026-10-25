#include <gtest/gtest.h>

#include <app/ringbuffer.h>

#include <memory>
#include <utility>

namespace app {
	namespace {
		template<int Capacity>
		concept ValidRingBufferCapacity = requires {
			typename RingBuffer<int, Capacity>;
		};

		static_assert(ValidRingBufferCapacity<1>);
		static_assert(!ValidRingBufferCapacity<0>);
		static_assert(!ValidRingBufferCapacity<-1>);

		constexpr bool supportsConstantEvaluation() {
			RingBuffer<int, 2> buffer;
			if (!buffer.tryPush(1) || !buffer.tryPush(2) || buffer.tryPush(3)) {
				return false;
			}
			if (buffer.front() != 1 || !buffer.full()) {
				return false;
			}
			buffer.pop();
			buffer.clear();
			return buffer.empty() && buffer.size() == 0 && buffer.capacity() == 2;
		}

		static_assert(supportsConstantEvaluation());

	}

	TEST(RingBufferTest, reportsEmptyAndFullState) {
		// Given
		RingBuffer<int, 2> buffer;

		// When / Then
		EXPECT_TRUE(buffer.empty());
		EXPECT_FALSE(buffer.full());
		EXPECT_EQ(buffer.size(), 0);
		EXPECT_EQ(buffer.capacity(), 2);

		EXPECT_TRUE(buffer.tryPush(1));
		EXPECT_FALSE(buffer.empty());
		EXPECT_FALSE(buffer.full());
		EXPECT_EQ(buffer.size(), 1);

		EXPECT_TRUE(buffer.tryPush(2));
		EXPECT_FALSE(buffer.empty());
		EXPECT_TRUE(buffer.full());
		EXPECT_EQ(buffer.size(), 2);
	}

	TEST(RingBufferTest, returnsValuesInFirstInFirstOutOrder) {
		// Given
		RingBuffer<int, 3> buffer;
		ASSERT_TRUE(buffer.tryPush(10));
		ASSERT_TRUE(buffer.tryPush(20));
		ASSERT_TRUE(buffer.tryPush(30));

		// When / Then
		EXPECT_EQ(buffer.front(), 10);
		buffer.pop();
		EXPECT_EQ(buffer.front(), 20);
		buffer.pop();
		EXPECT_EQ(buffer.front(), 30);
		buffer.pop();
		EXPECT_TRUE(buffer.empty());
	}

	TEST(RingBufferTest, preservesOrderAfterWrappingAroundStorage) {
		// Given
		RingBuffer<int, 3> buffer;
		ASSERT_TRUE(buffer.tryPush(1));
		ASSERT_TRUE(buffer.tryPush(2));
		buffer.pop();
		ASSERT_TRUE(buffer.tryPush(3));
		ASSERT_TRUE(buffer.tryPush(4));

		// When / Then
		EXPECT_EQ(buffer.front(), 2);
		buffer.pop();
		EXPECT_EQ(buffer.front(), 3);
		buffer.pop();
		EXPECT_EQ(buffer.front(), 4);
	}

	TEST(RingBufferTest, rejectsOverflowWithoutReplacingExistingValues) {
		// Given
		RingBuffer<int, 2> buffer;
		ASSERT_TRUE(buffer.tryPush(1));
		ASSERT_TRUE(buffer.tryPush(2));

		// When
		const auto inserted = buffer.tryPush(3);

		// Then
		EXPECT_FALSE(inserted);
		EXPECT_EQ(buffer.size(), 2);
		EXPECT_EQ(buffer.front(), 1);
		buffer.pop();
		EXPECT_EQ(buffer.front(), 2);
	}

	TEST(RingBufferTest, clearDiscardsValuesAndAllowsReuse) {
		// Given
		RingBuffer<int, 2> buffer;
		ASSERT_TRUE(buffer.tryPush(1));
		ASSERT_TRUE(buffer.tryPush(2));

		// When
		buffer.clear();

		// Then
		EXPECT_TRUE(buffer.empty());
		EXPECT_EQ(buffer.size(), 0);
		EXPECT_TRUE(buffer.tryPush(3));
		EXPECT_EQ(buffer.front(), 3);
	}

	TEST(RingBufferTest, providesConstAccessToFrontValue) {
		// Given
		RingBuffer<int, 1> buffer;
		ASSERT_TRUE(buffer.tryPush(42));
		const auto& constBuffer = buffer;

		// When
		const auto& value = constBuffer.front();

		// Then
		EXPECT_EQ(value, 42);
	}

	TEST(RingBufferTest, storesMoveOnlyValues) {
		// Given
		RingBuffer<std::unique_ptr<int>, 2> buffer;
		auto value = std::make_unique<int>(42);

		// When
		const auto inserted = buffer.tryPush(std::move(value));

		// Then
		EXPECT_TRUE(inserted);
		EXPECT_EQ(value, nullptr);
		ASSERT_NE(buffer.front(), nullptr);
		EXPECT_EQ(*buffer.front(), 42);
		buffer.pop();
		EXPECT_TRUE(buffer.empty());
	}

}
