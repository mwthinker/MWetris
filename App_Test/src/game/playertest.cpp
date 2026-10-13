#include <gtest/gtest.h>

#include <app/controlprofiles.h>
#include <app/game/devicemanager.h>
#include <app/game/player.h>

#include <SDL3/SDL.h>

namespace app::game {

	namespace {

		SDL_Event keyboardEvent(SDL_Keycode key, SDL_EventType type) {
			SDL_Event event{};
			event.key = {
				.type = type,
				.key = key,
			};
			return event;
		}

		int countMoves(Player& player, tetris::Move expectedMove) {
			int count = 0;
			mw::signals::ScopedConnection connection = player.playerBoardUpdate.connect([&](const PlayerBoardEvent& event) {
				if (const auto* move = std::get_if<UpdateMove>(&event); move && move->move == expectedMove) {
					++count;
				}
			});
			player.update(0.0);
			return count;
		}

	}

	TEST(PlayerTest, initializesFromPersistentDeviceDefaultAndRejectsInvalidProfiles) {
		auto profiles = std::make_shared<ControlProfiles>();
		DeviceManager devices;
		auto device = devices.getDefaultDevice1();
		ASSERT_TRUE(profiles->assignProfile(device->getGuid(), ControlProfiles::FastProfileId));

		auto player = createHumanPlayer(device, profiles);

		EXPECT_EQ(player->getDevice(), device);
		EXPECT_EQ(player->getProfileId(), ControlProfiles::FastProfileId);
		EXPECT_FALSE(player->setProfileId(ProfileId{"missing"}));
		EXPECT_EQ(player->getProfileId(), ControlProfiles::FastProfileId);
		EXPECT_FALSE(player->setDevice(nullptr));
		EXPECT_EQ(player->getDevice(), device);
	}

	TEST(PlayerTest, deviceReplacementRetainsProfileChangesInputSourceAndClearsHeldRepeat) {
		auto profiles = std::make_shared<ControlProfiles>();
		DeviceManager devices;
		auto device1 = devices.getDefaultDevice1();
		auto device2 = devices.getDefaultDevice2();
		auto player = createHumanPlayer(device1, profiles);
		ASSERT_TRUE(player->setProfileId(ControlProfiles::FastProfileId));

		devices.eventUpdate(keyboardEvent(SDLK_LEFT, SDL_EVENT_KEY_DOWN));
		EXPECT_EQ(countMoves(*player, tetris::Move::Left), 1);
		devices.tick();
		player->update(0.09);

		ASSERT_TRUE(player->setDevice(device2));
		EXPECT_EQ(player->getProfileId(), ControlProfiles::FastProfileId);
		EXPECT_EQ(countMoves(*player, tetris::Move::Left), 0);
		devices.eventUpdate(keyboardEvent(SDLK_A, SDL_EVENT_KEY_DOWN));
		EXPECT_EQ(countMoves(*player, tetris::Move::Left), 1);

		devices.tick();
		player->update(0.09);
		ASSERT_TRUE(player->setDevice(device1));
		EXPECT_EQ(countMoves(*player, tetris::Move::Left), 0);
		int repeatedMoves = 0;
		mw::signals::ScopedConnection connection = player->playerBoardUpdate.connect([&](const PlayerBoardEvent& event) {
			if (const auto* move = std::get_if<UpdateMove>(&event); move && move->move == tetris::Move::Left) {
				++repeatedMoves;
			}
		});
		player->update(0.02);
		EXPECT_EQ(repeatedMoves, 0);
		player->update(0.08);
		EXPECT_EQ(repeatedMoves, 1);
	}

	TEST(PlayerTest, deletedRuntimeProfileFallsBackToBalancedOnUpdate) {
		auto profiles = std::make_shared<ControlProfiles>();
		DeviceManager devices;
		auto customProfile = profiles->duplicateProfile(
			devices.getDefaultDevice1()->getGuid(),
			ControlProfiles::BalancedProfileId,
			"Custom",
			{200, 40});
		ASSERT_TRUE(customProfile);
		auto player = createHumanPlayer(devices.getDefaultDevice1(), profiles);
		EXPECT_EQ(player->getProfileId(), *customProfile);

		ASSERT_TRUE(profiles->deleteProfile(*customProfile));
		player->update(0.0);

		EXPECT_EQ(player->getProfileId(), ControlProfiles::BalancedProfileId);
	}

	TEST(PlayerTest, aiAndRemotePlayersHaveNoRuntimeControls) {
		auto device = std::make_shared<Device>();
		auto ai = createAiPlayer(tetris::Ai{});
		auto remote = createRemotePlayer();

		for (const auto& player : {ai, remote}) {
			EXPECT_FALSE(player->getDevice());
			EXPECT_FALSE(player->setDevice(device));
			EXPECT_FALSE(player->getProfileId());
			EXPECT_FALSE(player->setProfileId(ControlProfiles::FastProfileId));
		}
	}

	TEST(PlayerTest, sharedProfileEditsUpdateTimingForEveryHuman) {
		auto profiles = std::make_shared<ControlProfiles>();
		DeviceManager devices;
		auto device = devices.getDefaultDevice1();
		auto customProfile = profiles->duplicateProfile(
			device->getGuid(),
			ControlProfiles::BalancedProfileId,
			"Shared",
			{300, 40});
		ASSERT_TRUE(customProfile);
		auto player1 = createHumanPlayer(device, profiles);
		auto player2 = createHumanPlayer(device, profiles);
		ASSERT_EQ(player1->getProfileId(), *customProfile);
		ASSERT_EQ(player2->getProfileId(), *customProfile);

		devices.eventUpdate(keyboardEvent(SDLK_LEFT, SDL_EVENT_KEY_DOWN));
		EXPECT_EQ(countMoves(*player1, tetris::Move::Left), 1);
		EXPECT_EQ(countMoves(*player2, tetris::Move::Left), 1);
		devices.tick();
		player1->update(0.15);
		player2->update(0.15);

		ASSERT_TRUE(profiles->editProfile(*customProfile, "Shared Edited", {100, 40}));
		int player1Moves = 0;
		int player2Moves = 0;
		mw::signals::ScopedConnection connection1 = player1->playerBoardUpdate.connect([&](const PlayerBoardEvent& event) {
			if (const auto* move = std::get_if<UpdateMove>(&event); move && move->move == tetris::Move::Left) {
				++player1Moves;
			}
		});
		mw::signals::ScopedConnection connection2 = player2->playerBoardUpdate.connect([&](const PlayerBoardEvent& event) {
			if (const auto* move = std::get_if<UpdateMove>(&event); move && move->move == tetris::Move::Left) {
				++player2Moves;
			}
		});
		player1->update(0.11);
		player2->update(0.11);

		EXPECT_EQ(player1Moves, 1);
		EXPECT_EQ(player2Moves, 1);
	}

}
