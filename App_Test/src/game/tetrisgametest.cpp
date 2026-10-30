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

}
