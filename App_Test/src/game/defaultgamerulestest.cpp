#include <gtest/gtest.h>

#include <app/game/defaultgamerules.h>

#include <vector>

namespace app::game {

	namespace {

		PlayerPtr createPlayerClearingRows(int rowsToClear, int clearedRows = 0) {
			constexpr int Width = 4;
			constexpr int Height = 8;
			std::vector<tetris::BlockType> board(Width * Height, tetris::BlockType::Empty);
			for (int row = 0; row < rowsToClear; ++row) {
				for (int column = 1; column < Width; ++column) {
					board[row * Width + column] = tetris::BlockType::J;
				}
			}

			auto player = createAiPlayer(
				tetris::Ai{},
				DefaultPlayerData{},
				tetris::TetrisBoard{
					board,
					Width,
					Height,
					tetris::Block{tetris::BlockType::I, 0, 0},
					tetris::BlockType::O});
			player->setClearedRows(clearedRows);
			return player;
		}

		DefaultPlayerData clearRows(int rowsToClear, int clearedRows = 0) {
			auto player = createPlayerClearingRows(rowsToClear, clearedRows);
			DefaultGameRules rules;
			rules.createGame({player});
			player->updateMove(tetris::Move::DownGravity);
			return std::get<DefaultPlayerData>(player->getPlayerData());
		}

	}

	TEST(DefaultGameRulesTest, startsAtLevelZero) {
		// Given
		auto player = createPlayerClearingRows(0);
		DefaultGameRules rules;

		// When
		rules.createGame({player});

		// Then
		const auto data = std::get<DefaultPlayerData>(player->getPlayerData());
		EXPECT_EQ(data.level, 0);
	}

	TEST(DefaultGameRulesTest, awardsFortyPointsForOneClearedRow) {
		// Given
		constexpr int RowsToClear{1};

		// When
		const auto data = clearRows(RowsToClear);

		// Then
		EXPECT_EQ(data.points, 40);
	}

	TEST(DefaultGameRulesTest, awardsOneHundredPointsForTwoClearedRows) {
		// Given
		constexpr int RowsToClear{2};

		// When
		const auto data = clearRows(RowsToClear);

		// Then
		EXPECT_EQ(data.points, 100);
	}

	TEST(DefaultGameRulesTest, awardsThreeHundredPointsForThreeClearedRows) {
		// Given
		constexpr int RowsToClear{3};

		// When
		const auto data = clearRows(RowsToClear);

		// Then
		EXPECT_EQ(data.points, 300);
	}

	TEST(DefaultGameRulesTest, awardsTwelveHundredPointsForFourClearedRows) {
		// Given
		constexpr int RowsToClear{4};

		// When
		const auto data = clearRows(RowsToClear);

		// Then
		EXPECT_EQ(data.points, 1200);
	}

	TEST(DefaultGameRulesTest, multipliesLineClearPointsByLevelPlusOne) {
		// Given
		constexpr int RowsToClear{3};
		constexpr int Level{2};

		// When
		const int points{calculateLineClearPoints(RowsToClear, Level)};

		// Then
		EXPECT_EQ(points, 900);
	}

	TEST(DefaultGameRulesTest, addsLineClearPointsToCurrentScore) {
		// Given
		constexpr int CurrentPoints{1200};

		// When
		const int points{CurrentPoints + calculateLineClearPoints(2, 1)};

		// Then
		EXPECT_EQ(points, 1400);
	}

	TEST(DefaultGameRulesTest, scoresAtOldLevelBeforeCrossingBoundary) {
		// Given
		constexpr int ClearedRows{9};

		// When
		const auto data = clearRows(1, ClearedRows);

		// Then
		EXPECT_EQ(data.points, 40);
		EXPECT_EQ(data.level, 1);
	}

	TEST(DefaultGameRulesTest, usesFortyEightFrameGravityAtLevelZero) {
		// Given
		constexpr int Level{0};

		// When
		const double gravity{calculateGravity(Level)};

		// Then
		EXPECT_DOUBLE_EQ(gravity, 60.0988 / 48);
	}

	TEST(DefaultGameRulesTest, keepsFiveFrameGravityFromLevelsTenThroughTwelve) {
		// Given
		constexpr double ExpectedGravity{60.0988 / 5};

		// When
		const std::array gravity{
			calculateGravity(10),
			calculateGravity(11),
			calculateGravity(12)
		};

		// Then
		EXPECT_EQ(gravity, (std::array{ExpectedGravity, ExpectedGravity, ExpectedGravity}));
	}

	TEST(DefaultGameRulesTest, capsGravityAtOneFrameForLevelsTwentyNineAndHigher) {
		// Given
		constexpr double ExpectedGravity{60.0988};

		// When
		const std::array gravity{
			calculateGravity(29),
			calculateGravity(40)
		};

		// Then
		EXPECT_EQ(gravity, (std::array{ExpectedGravity, ExpectedGravity}));
	}

}
