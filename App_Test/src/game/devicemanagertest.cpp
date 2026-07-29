#include <gtest/gtest.h>

#include <app/game/devicemanager.h>

namespace app::game {

	class DeviceManagerTest : public ::testing::Test {
	protected:

		DeviceManagerTest() {}

		~DeviceManagerTest() override {}

		void SetUp() override {
			controlProfiles = std::make_shared<ControlProfiles>();
			deviceManager = std::make_shared<game::DeviceManager>(controlProfiles);
		}

		void TearDown() override {}

		std::shared_ptr<ControlProfiles> controlProfiles;
		std::shared_ptr<DeviceManager> deviceManager;
	};

	TEST_F(DeviceManagerTest, test) {
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

	TEST_F(DeviceManagerTest, updatesOnlyDevicesUsingChangedTiming) {
		auto device1 = deviceManager->getDefaultDevice1();
		auto device2 = deviceManager->getDefaultDevice2();
		ASSERT_TRUE(device1);
		ASSERT_TRUE(device2);
		EXPECT_EQ(device1->getInputTiming(), (InputTiming{167, 33}));
		EXPECT_EQ(device2->getInputTiming(), (InputTiming{167, 33}));
		EXPECT_EQ(device1->getInputTimingRevision(), 1);
		EXPECT_EQ(device2->getInputTimingRevision(), 1);

		ASSERT_TRUE(controlProfiles->assignProfile(device1->getGuid(), ControlProfiles::FastProfileId));
		EXPECT_EQ(device1->getInputTiming(), (InputTiming{100, 16}));
		EXPECT_EQ(device1->getInputTimingRevision(), 2);
		EXPECT_EQ(device2->getInputTiming(), (InputTiming{167, 33}));
		EXPECT_EQ(device2->getInputTimingRevision(), 1);

		auto customProfile = controlProfiles->duplicateProfile(
			device1->getGuid(),
			ControlProfiles::FastProfileId,
			"Custom",
			{120, 20});
		ASSERT_TRUE(customProfile);
		EXPECT_EQ(device1->getInputTiming(), (InputTiming{120, 20}));
		EXPECT_EQ(device1->getInputTimingRevision(), 3);
		EXPECT_EQ(device2->getInputTimingRevision(), 1);

		ASSERT_TRUE(controlProfiles->editProfile(*customProfile, "Renamed", {130, 25}));
		EXPECT_EQ(device1->getInputTiming(), (InputTiming{130, 25}));
		EXPECT_EQ(device1->getInputTimingRevision(), 4);
		EXPECT_EQ(device2->getInputTiming(), (InputTiming{167, 33}));
		EXPECT_EQ(device2->getInputTimingRevision(), 1);

		ASSERT_TRUE(controlProfiles->editProfile(*customProfile, "Renamed Again", {130, 25}));
		EXPECT_EQ(device1->getInputTimingRevision(), 4);
		EXPECT_EQ(device2->getInputTimingRevision(), 1);
	}

}
