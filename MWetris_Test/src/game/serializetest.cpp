#include <gtest/gtest.h>

#include <game/serialize.h>

namespace mwetris::game {

	class SerilizeTest : public ::testing::Test {
	protected:

		SerilizeTest() {}

		~SerilizeTest() override {}

		void SetUp() override {
			
		}

		void TearDown() override {}
		
	};

	TEST_F(SerilizeTest, defaultDevicesIsSetupCorrectly) {
	}

}
