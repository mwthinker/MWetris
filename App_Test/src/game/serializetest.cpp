#include <gtest/gtest.h>

#include <app/game/serialize.h>
#include <app/game/devicemanager.h>
#include <app/game/player.h>

namespace app::game {

	class SerilizeTest : public ::testing::Test {
	protected:

		SerilizeTest() {}

		~SerilizeTest() override {}

		void SetUp() override {
			
		}

		void TearDown() override {}
		
	};

	TEST_F(SerilizeTest, loadedHumanUsesSharedCatalogAndPersistentDeviceDefault) {
		clearSavedGame();
		auto profiles = std::make_shared<ControlProfiles>();
		DeviceManager devices;
		auto device = devices.getDefaultDevice1();
		ASSERT_TRUE(profiles->assignProfile(device->getGuid(), ControlProfiles::FastProfileId));
		auto savedPlayer = createHumanPlayer(device, profiles);
		ASSERT_TRUE(savedPlayer->setProfileId(ControlProfiles::ClassicProfileId));
		saveGame(*savedPlayer);

		auto loadedPlayer = loadGame(device, profiles);

		ASSERT_TRUE(loadedPlayer);
		EXPECT_EQ(loadedPlayer->getDevice(), device);
		EXPECT_EQ(loadedPlayer->getProfileId(), ControlProfiles::FastProfileId);
		clearSavedGame();
	}

}
