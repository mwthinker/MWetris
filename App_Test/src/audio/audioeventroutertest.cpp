#include <gtest/gtest.h>

#include <app/audio/audioeventrouter.h>
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

		void expectEffects(const std::vector<SoundEffect>& actual, std::initializer_list<SoundEffect> expected) {
			EXPECT_EQ(actual, std::vector<SoundEffect>(expected));
		}
	}

	TEST(AudioEventRouterTest, confirmsOnlySuccessfulLocalMoves) {
		AudioEventRouter router;
		auto player = humanPlayer();
		expectEffects(router.route(*player, game::UpdateMove{tetris::Move::Left}), {});
		expectEffects(router.route(*player, game::TetrisBoardEvent{tetris::BoardEvent::PlayerMovesBlock, 0}), {SoundEffect::Move});
		expectEffects(router.route(*player, game::UpdateMove{tetris::Move::Right}), {});
		expectEffects(router.route(*player, game::UpdateMove{tetris::Move::DownGravity}), {});
		expectEffects(router.route(*player, game::TetrisBoardEvent{tetris::BoardEvent::GravityMovesBlock, 0}), {});
		expectEffects(router.route(*player, game::TetrisBoardEvent{tetris::BoardEvent::PlayerMovesBlock, 0}), {});
		expectEffects(router.route(*player, game::UpdateMove{tetris::Move::DownGround}), {});
		expectEffects(router.route(*player, game::TetrisBoardEvent{tetris::BoardEvent::PlayerMovesBlock, 0}), {SoundEffect::HardDrop});
	}

	TEST(AudioEventRouterTest, mapsLockClearsLevelAndGarbage) {
		AudioEventRouter router;
		auto player = humanPlayer();
		expectEffects(router.route(*player, game::TetrisBoardEvent{tetris::BoardEvent::BlockCollision, 0}), {});
		expectEffects(router.route(*player, game::TetrisBoardEvent{tetris::BoardEvent::CurrentBlockUpdated, 0}), {SoundEffect::Lock});
		for (int rows = 1; rows <= 3; ++rows) {
			expectEffects(router.route(*player, game::TetrisBoardEvent{tetris::BoardEvent::RowsRemoved, rows}), {SoundEffect::LineClear});
		}
		expectEffects(router.route(*player, game::TetrisBoardEvent{tetris::BoardEvent::RowsRemoved, 4}), {SoundEffect::LineClear4});
		expectEffects(router.route(*player, game::UpdatePlayerData{game::DefaultPlayerData{.level = 2}}), {});
		expectEffects(router.route(*player, game::UpdatePlayerData{game::DefaultPlayerData{.level = 3}}), {SoundEffect::LevelUp});
		expectEffects(router.route(*player, game::ExternalRows{{tetris::BlockType::I}}), {SoundEffect::Garbage});
	}

	TEST(AudioEventRouterTest, mapsGameLifecycleAndReset) {
		AudioEventRouter router;
		auto player = humanPlayer();
		expectEffects(router.route(TetrisEvent{CreateGameEvent{}}), {});
		expectEffects(router.route(TetrisEvent{game::GamePause{.countDown = 3, .pause = true}}), {SoundEffect::Countdown});
		expectEffects(router.route(TetrisEvent{game::GamePause{.countDown = 2, .pause = true}}), {SoundEffect::Countdown});
		expectEffects(router.route(TetrisEvent{game::GamePause{.countDown = 0, .pause = false}}), {SoundEffect::Go});
		expectEffects(router.route(TetrisEvent{game::GamePause{.countDown = 0, .pause = true}}), {SoundEffect::Pause});
		expectEffects(router.route(TetrisEvent{game::GamePause{.countDown = 0, .pause = false}}), {SoundEffect::Resume});
		expectEffects(router.route(TetrisEvent{game::GameOver{player}}), {SoundEffect::GameOver});
		expectEffects(router.route(TetrisEvent{CreateGameEvent{}}), {});
		expectEffects(router.route(TetrisEvent{game::GamePause{.countDown = 0, .pause = false}}), {SoundEffect::Go});
	}

	TEST(AudioEventRouterTest, suppressesAiAndRemotePlayers) {
		AudioEventRouter router;
		auto ai = game::createAiPlayer(tetris::Ai{"-1*holes"});
		auto remote = game::createRemotePlayer();
		expectEffects(router.route(*ai, game::UpdateMove{tetris::Move::Left}), {});
		expectEffects(router.route(*remote, game::ExternalRows{{tetris::BlockType::I}}), {});
		expectEffects(router.route(TetrisEvent{game::GameOver{ai}}), {});
	}

}
