#include <gtest/gtest.h>

#include <app/game/serialize.h>
#include <app/game/serializehighscore.h>
#include <app/game/devicemanager.h>
#include <app/game/player.h>

#include <filesystem>

namespace app::game {

	class SerilizeTest : public ::testing::Test {
	protected:

		SerilizeTest() {}

		~SerilizeTest() override {}

		void SetUp() override {
			clearSavedGame();
			std::filesystem::remove("highScore.mw");
		}

		void TearDown() override {
			clearSavedGame();
			std::filesystem::remove("highScore.mw");
		}
		
	};

	TEST_F(SerilizeTest, loadedHumanUsesSharedCatalogAndPersistentDeviceDefault) {
		// Given
		auto profiles = std::make_shared<ControlProfiles>();
		DeviceManager devices;
		auto device = devices.getDefaultDevice1();
		ASSERT_TRUE(profiles->assignProfile(device->getGuid(), ControlProfiles::FastProfileId));
		auto savedPlayer = createHumanPlayer(device, profiles);
		ASSERT_TRUE(savedPlayer->setProfileId(ControlProfiles::ClassicProfileId));
		saveGame(*savedPlayer);

		// When
		auto loadedPlayer = loadGame(device, profiles);

		// Then
		ASSERT_TRUE(loadedPlayer);
		EXPECT_EQ(loadedPlayer->getDevice(), device);
		EXPECT_EQ(loadedPlayer->getProfileId(), ControlProfiles::FastProfileId);
	}

	TEST_F(SerilizeTest, derivesZeroBasedLevelFromClearedRows) {
		// Given
		auto profiles = std::make_shared<ControlProfiles>();
		DeviceManager devices;
		auto savedPlayer = createHumanPlayer(
			devices.getDefaultDevice1(),
			profiles,
			DefaultPlayerData{.level = 3, .points = 1234});
		savedPlayer->setClearedRows(27);
		saveGame(*savedPlayer);

		// When
		auto loadedPlayer = loadGame(devices.getDefaultDevice1(), profiles);

		// Then
		ASSERT_TRUE(loadedPlayer);
		const auto data = std::get<DefaultPlayerData>(loadedPlayer->getPlayerData());
		EXPECT_EQ(data.level, 2);
	}

	TEST_F(SerilizeTest, preservesZeroBasedLevelThroughRoundTrip) {
		// Given
		auto profiles = std::make_shared<ControlProfiles>();
		DeviceManager devices;
		auto savedPlayer = createHumanPlayer(
			devices.getDefaultDevice1(),
			profiles,
			DefaultPlayerData{.level = 2, .points = 1234});
		savedPlayer->setClearedRows(27);

		// When
		saveGame(*savedPlayer);
		auto roundTrippedPlayer = loadGame(devices.getDefaultDevice1(), profiles);

		// Then
		ASSERT_TRUE(roundTrippedPlayer);
		EXPECT_EQ(std::get<DefaultPlayerData>(roundTrippedPlayer->getPlayerData()).level, 2);
	}

	TEST_F(SerilizeTest, restoresStartingLevelFromSavedGame) {
		// Given
		auto profiles = std::make_shared<ControlProfiles>();
		DeviceManager devices;
		auto savedPlayer = createHumanPlayer(
			devices.getDefaultDevice1(),
			profiles,
			DefaultPlayerData{.level = 9, .points = 1234, .startLevel = 9});
		savedPlayer->setClearedRows(50);
		saveGame(*savedPlayer);

		// When
		auto loadedPlayer = loadGame(devices.getDefaultDevice1(), profiles);

		// Then
		ASSERT_TRUE(loadedPlayer);
		const auto data = std::get<DefaultPlayerData>(loadedPlayer->getPlayerData());
		EXPECT_EQ(data.startLevel, 9);
		EXPECT_EQ(data.level, 9);
	}

	TEST_F(SerilizeTest, derivesHighScoreLevelFromClearedRows) {
		// Given
		saveHighScore("Legacy", 1234, 27, 3);

		// When
		const auto results = loadHighScore();

		// Then
		ASSERT_FALSE(results.empty());
		EXPECT_EQ(results.front().name, "Legacy");
		EXPECT_EQ(results.front().level, 2);
	}

	TEST_F(SerilizeTest, preservesZeroBasedHighScoreLevelThroughRoundTrip) {
		// Given
		saveHighScore("Current", 1234, 100, 10, 9);

		// When
		const auto roundTrippedResults = loadHighScore();

		// Then
		ASSERT_FALSE(roundTrippedResults.empty());
		EXPECT_EQ(roundTrippedResults.front().name, "Current");
		EXPECT_EQ(roundTrippedResults.front().level, 10);
		EXPECT_EQ(roundTrippedResults.front().startLevel, 9);
	}

}
