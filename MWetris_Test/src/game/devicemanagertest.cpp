#include <gtest/gtest.h>

#include <game/devicemanager.h>

namespace mwetris::game {

	class DeviceManagerTestTest : public ::testing::Test {
	protected:

		DeviceManagerTestTest() {}

		~DeviceManagerTestTest() override {}

		void SetUp() override {
			deviceManager = std::make_shared<game::DeviceManager>();
		}

		void TearDown() override {}

		std::shared_ptr<DeviceManager> deviceManager;
	};

	TEST_F(DeviceManagerTestTest, defaultDevicesIsSetupCorrectly) {
		// Given
		auto device1 = deviceManager->getDefaultDevice1();
		auto device2 = deviceManager->getDefaultDevice2();

		// When/Then
		ASSERT_TRUE(device1);
		ASSERT_TRUE(device1->isConnected());

		ASSERT_TRUE(device2);
		ASSERT_TRUE(device2->isConnected());
		
		ASSERT_NE(device1, device2);
		ASSERT_NE(device1->getGuid(), device2->getGuid());
		ASSERT_NE(device1->getName(), device2->getName());
	}

}
