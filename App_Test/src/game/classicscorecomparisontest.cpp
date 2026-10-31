#include <gtest/gtest.h>

#include <app/game/classicscorecomparison.h>

namespace app::game {

	TEST(ClassicScoreComparisonTest, reversesDifferencesWhenArgumentOrderIsReversed) {
		// Given
		const ClassicScoreEntry leader{.points = 2400, .level = 0, .clearedRows = 0, .startLevel = 0};
		const ClassicScoreEntry trailer{.points = 1200, .level = 0, .clearedRows = 0, .startLevel = 0};

		// When
		const auto leadingComparison = calculateClassicScoreComparison(leader, trailer);
		const auto trailingComparison = calculateClassicScoreComparison(trailer, leader);

		// Then
		EXPECT_EQ(leadingComparison.pointDifference, 1200);
		EXPECT_DOUBLE_EQ(leadingComparison.tetrisDifference, 1.0);
		EXPECT_EQ(trailingComparison.pointDifference, -1200);
		EXPECT_DOUBLE_EQ(trailingComparison.tetrisDifference, -1.0);
	}

	TEST(ClassicScoreComparisonTest, returnsZeroDifferencesForTiedScores) {
		// Given
		const ClassicScoreEntry player{.points = 5000, .level = 3, .clearedRows = 30, .startLevel = 0};
		const ClassicScoreEntry opponent{.points = 5000, .level = 8, .clearedRows = 80, .startLevel = 0};

		// When
		const auto comparison = calculateClassicScoreComparison(player, opponent);

		// Then
		EXPECT_EQ(comparison.pointDifference, 0);
		EXPECT_DOUBLE_EQ(comparison.tetrisDifference, 0.0);
	}

	TEST(ClassicScoreComparisonTest, returnsFractionOfTetrisAtTrailingPlayersLevel) {
		// Given
		const ClassicScoreEntry leader{.points = 5400, .level = 4, .clearedRows = 40, .startLevel = 0};
		const ClassicScoreEntry trailer{.points = 2400, .level = 4, .clearedRows = 40, .startLevel = 0};

		// When
		const auto comparison = calculateClassicScoreComparison(leader, trailer);

		// Then
		EXPECT_DOUBLE_EQ(comparison.tetrisDifference, 0.5);
	}

	TEST(ClassicScoreComparisonTest, advancesLevelAfterCrossingFirstTransition) {
		// Given
		const ClassicScoreEntry leader{.points = 3600, .level = 0, .clearedRows = 0, .startLevel = 0};
		const ClassicScoreEntry trailer{.points = 0, .level = 0, .clearedRows = 8, .startLevel = 0};

		// When
		const auto comparison = calculateClassicScoreComparison(leader, trailer);

		// Then
		EXPECT_DOUBLE_EQ(comparison.tetrisDifference, 2.0);
	}

	TEST(ClassicScoreComparisonTest, advancesLevelAfterCrossingSubsequentTransition) {
		// Given
		const ClassicScoreEntry leader{.points = 4200, .level = 0, .clearedRows = 0, .startLevel = 0};
		const ClassicScoreEntry trailer{.points = 0, .level = 1, .clearedRows = 18, .startLevel = 0};

		// When
		const auto comparison = calculateClassicScoreComparison(leader, trailer);

		// Then
		EXPECT_DOUBLE_EQ(comparison.tetrisDifference, 1.5);
	}

}
