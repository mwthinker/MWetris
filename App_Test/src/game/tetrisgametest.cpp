#include <gtest/gtest.h>

#include <app/game/player.h>
#include <app/game/tetrisgame.h>

namespace app::game {

	TEST(TetrisGameTest, emitsGameOverForPlayer) {
		// Given
		TetrisGame game;
		auto player = createRemotePlayer();
		int eventCount = 0;
		PlayerPtr eventPlayer;
		mw::signals::ScopedConnection connection = game.gameOverEvent.connect([&](const GameOver& gameOver) {
			++eventCount;
			eventPlayer = gameOver.player;
		});
		game.createGame({player});

		// When
		player->updateMove(tetris::Move::GameOver);

		// Then
		EXPECT_EQ(eventCount, 1);
		EXPECT_EQ(eventPlayer, player);
	}

	TEST(TetrisGameTest, ignoresNonGameOverBoardEvents) {
		// Given
		TetrisGame game;
		auto player = createRemotePlayer();
		int eventCount = 0;
		mw::signals::ScopedConnection connection = game.gameOverEvent.connect([&](const GameOver&) {
			++eventCount;
		});
		game.createGame({player});

		// When
		player->updateMove(tetris::Move::Left);

		// Then
		EXPECT_EQ(eventCount, 0);
	}

	TEST(TetrisGameTest, disconnectsPlayersFromPreviousGame) {
		// Given
		TetrisGame game;
		auto previousPlayer = createRemotePlayer();
		auto currentPlayer = createRemotePlayer();
		int eventCount = 0;
		PlayerPtr eventPlayer;
		mw::signals::ScopedConnection connection = game.gameOverEvent.connect([&](const GameOver& gameOver) {
			++eventCount;
			eventPlayer = gameOver.player;
		});
		game.createGame({previousPlayer});
		game.createGame({currentPlayer});

		// When
		previousPlayer->updateMove(tetris::Move::GameOver);
		currentPlayer->updateMove(tetris::Move::GameOver);

		// Then
		EXPECT_EQ(eventCount, 1);
		EXPECT_EQ(eventPlayer, currentPlayer);
	}

	TEST(TetrisGameTest, countsDownBeforeStartingSinglePlayerGame) {
		// Given
		TetrisGame game;
		std::vector<GamePause> events;
		mw::signals::ScopedConnection connection = game.gamePauseEvent.connect([&](const GamePause& event) {
			events.push_back(event);
		});

		// When
		game.createGame({createRemotePlayer()});
		game.update(1.0);
		game.update(1.0);
		game.update(1.0);

		// Then
		ASSERT_EQ(events.size(), 4);
		EXPECT_EQ(events[0].countDown, 3);
		EXPECT_EQ(events[1].countDown, 2);
		EXPECT_EQ(events[2].countDown, 1);
		EXPECT_EQ(events[3].action, GamePauseAction::CountdownFinished);
		EXPECT_FALSE(events[3].pause);
		EXPECT_FALSE(game.isPaused());
	}

}
