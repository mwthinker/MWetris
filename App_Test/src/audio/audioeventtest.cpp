#include <gtest/gtest.h>

#include <app/audio/audioevent.h>
#include <app/controlprofiles.h>
#include <app/game/devicemanager.h>
#include <app/game/player.h>

namespace app::audio {

	namespace {
		game::PlayerPtr humanPlayer() {
			static game::DeviceManager devices;
			static auto profiles = std::make_shared<ControlProfiles>();
			return game::createHumanPlayer(devices.getDefaultDevice1(), profiles);
		}
	}

	TEST(AudioEventTest, mapsSuccessfulBoardActions) {
		// Given
		auto player = humanPlayer();

		// When / Then
		EXPECT_EQ(soundEffectFor(*player, game::TetrisBoardEvent{tetris::BoardEvent::PlayerMovedBlock, 0}), SoundEffect::Move);
		EXPECT_EQ(soundEffectFor(*player, game::TetrisBoardEvent{tetris::BoardEvent::PlayerRotatedBlock, 0}), SoundEffect::Move);
		EXPECT_EQ(soundEffectFor(*player, game::TetrisBoardEvent{tetris::BoardEvent::PlayerHardDroppedBlock, 0}), SoundEffect::HardDrop);
		EXPECT_EQ(soundEffectFor(*player, game::TetrisBoardEvent{tetris::BoardEvent::PlayerSoftDroppedBlock, 0}), std::nullopt);
	}

	TEST(AudioEventTest, mapsLockOutcomeAndPlayerEvents) {
		// Given
		auto player = humanPlayer();

		// When / Then
		EXPECT_EQ(soundEffectFor(*player, game::TetrisBoardEvent{tetris::BoardEvent::BlockLocked, 0}), SoundEffect::Lock);
		EXPECT_EQ(soundEffectFor(*player, game::TetrisBoardEvent{tetris::BoardEvent::BlockLocked, 1}), SoundEffect::LineClear);
		EXPECT_EQ(soundEffectFor(*player, game::TetrisBoardEvent{tetris::BoardEvent::BlockLocked, 4}), SoundEffect::LineClear4);
		EXPECT_EQ(soundEffectFor(*player, game::LevelUp{.previous = 2, .current = 3}), SoundEffect::LevelUp);
		EXPECT_EQ(soundEffectFor(*player, game::ExternalRows{{tetris::BlockType::I}}), SoundEffect::Garbage);
	}

	TEST(AudioEventTest, mapsExplicitGameLifecycleEvents) {
		// Given
		auto player = humanPlayer();

		// When / Then
		EXPECT_EQ(soundEffectFor(TetrisEvent{game::GamePause{.countDown = 3, .pause = true, .action = game::GamePauseAction::Countdown}}), SoundEffect::Countdown);
		EXPECT_EQ(soundEffectFor(TetrisEvent{game::GamePause{.countDown = 0, .pause = false, .action = game::GamePauseAction::CountdownFinished}}), SoundEffect::Go);
		EXPECT_EQ(soundEffectFor(TetrisEvent{game::GamePause{.countDown = 0, .pause = false, .action = game::GamePauseAction::Started}}), SoundEffect::Go);
		EXPECT_EQ(soundEffectFor(TetrisEvent{game::GamePause{.countDown = 0, .pause = true, .action = game::GamePauseAction::Paused}}), SoundEffect::Pause);
		EXPECT_EQ(soundEffectFor(TetrisEvent{game::GamePause{.countDown = 0, .pause = false, .action = game::GamePauseAction::Resumed}}), SoundEffect::Resume);
		EXPECT_EQ(soundEffectFor(TetrisEvent{game::GameOver{player}}), SoundEffect::GameOver);
	}

	TEST(AudioEventTest, ignoresAiAndRemotePlayerEvents) {
		// Given
		auto ai = game::createAiPlayer(tetris::Ai{"-1*holes"});
		auto remote = game::createRemotePlayer();
		game::TetrisBoardEvent event{tetris::BoardEvent::PlayerMovedBlock, 0};

		// When / Then
		EXPECT_EQ(soundEffectFor(*ai, event), std::nullopt);
		EXPECT_EQ(soundEffectFor(*remote, event), std::nullopt);
		EXPECT_EQ(soundEffectFor(TetrisEvent{game::GameOver{ai}}), std::nullopt);
	}

}
