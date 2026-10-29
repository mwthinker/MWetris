#include <gtest/gtest.h>

#include <app/game/tetrisboardcontroller.h>

#include <tetris/tetrisboard.h>

#include <algorithm>
#include <vector>

namespace app::game {

	namespace {

		std::vector<tetris::Move> collectMoves(
			TetrisBoardController& controller,
			tetris::TetrisBoard& board,
			Input input,
			double deltaTime) {
			std::vector<tetris::Move> moves;
			controller.update(board, input, deltaTime, [&](const PlayerBoardEvent& event) {
				if (const auto* update = std::get_if<UpdateMove>(&event)) {
					moves.push_back(update->move);
				}
			});
			return moves;
		}

	}

	TEST(TetrisBoardControllerTest, appliesTimingToBothHorizontalDirections) {
		for (const auto direction : {tetris::Move::Left, tetris::Move::Right}) {
			TetrisBoardController controller;
			controller.setInputTiming(0.2, 0.05);
			tetris::TetrisBoard board{40, 20, tetris::BlockType::O, tetris::BlockType::O};
			Input input;
			auto& key = direction == tetris::Move::Left ? input.left : input.right;
			key = true;
			ASSERT_TRUE(controller.enqueue({
				direction == tetris::Move::Left ? InputAction::Left : InputAction::Right, 0}));

			EXPECT_EQ(collectMoves(controller, board, input, 0.0), (std::vector{direction}));
			EXPECT_TRUE(collectMoves(controller, board, input, 0.19).empty());
			EXPECT_EQ(collectMoves(controller, board, input, 0.02), (std::vector{direction}));
			EXPECT_TRUE(collectMoves(controller, board, input, 0.04).empty());
			EXPECT_EQ(collectMoves(controller, board, input, 0.02), (std::vector{direction}));
		}
	}

	TEST(TetrisBoardControllerTest, timingChangeResetsBothHorizontalDirections) {
		TetrisBoardController controller;
		controller.setInputTiming(0.2, 0.05);
		tetris::TetrisBoard board{40, 20, tetris::BlockType::O, tetris::BlockType::O};
		Input input;
		input.left = true;
		input.right = true;
		ASSERT_TRUE(controller.enqueue({InputAction::Left, 0}));
		ASSERT_TRUE(controller.enqueue({InputAction::Right, 0}));
		EXPECT_EQ(
			collectMoves(controller, board, input, 0.0),
			(std::vector{tetris::Move::Left, tetris::Move::Right}));

		EXPECT_TRUE(collectMoves(controller, board, input, 0.15).empty());
		controller.setInputTiming(0.3, 0.05);
		EXPECT_TRUE(collectMoves(controller, board, input, 0.16).empty());
		EXPECT_EQ(
			collectMoves(controller, board, input, 0.15),
			(std::vector{tetris::Move::Left, tetris::Move::Right}));
	}

	TEST(TetrisBoardControllerTest, consumesEveryQueuedActionInFifoOrder) {
		// Given
		TetrisBoardController controller;
		tetris::TetrisBoard board{40, 20, tetris::BlockType::T, tetris::BlockType::O};
		const std::array actions{
			InputAction::Left,
			InputAction::Left,
			InputAction::Right,
			InputAction::Right,
			InputAction::SoftDrop,
			InputAction::SoftDrop,
			InputAction::Rotate,
			InputAction::Rotate,
			InputAction::HardDrop,
		};
		for (const auto action : actions) {
			ASSERT_TRUE(controller.enqueue({action, 0}));
		}

		// When
		const auto moves = collectMoves(controller, board, {}, 0.0);

		// Then
		EXPECT_EQ(moves, (std::vector{
			tetris::Move::Left,
			tetris::Move::Left,
			tetris::Move::Right,
			tetris::Move::Right,
			tetris::Move::Down,
			tetris::Move::Down,
			tetris::Move::RotateLeft,
			tetris::Move::RotateLeft,
			tetris::Move::DownGround,
		}));
		EXPECT_EQ(controller.pendingCommandCount(), 0);
	}

	TEST(TetrisBoardControllerTest, rejectsNewestCommandWhenQueueIsFull) {
		// Given
		TetrisBoardController controller;
		for (int index = 0; index < 16; ++index) {
			ASSERT_TRUE(controller.enqueue({InputAction::Left, static_cast<std::uint64_t>(index)}));
		}

		// When
		const auto accepted = controller.enqueue({InputAction::Right, 100});

		// Then
		EXPECT_FALSE(accepted);
		EXPECT_EQ(controller.pendingCommandCount(), 16);
		EXPECT_EQ(controller.overflowCount(), 1);
	}

	TEST(TetrisBoardControllerTest, softDropPressIsImmediateThenRepeatsWhileHeld) {
		// Given
		TetrisBoardController controller;
		tetris::TetrisBoard board{40, 20, tetris::BlockType::O, tetris::BlockType::O};
		Input input;
		input.down = true;
		ASSERT_TRUE(controller.enqueue({InputAction::SoftDrop, 0}));

		// When / Then
		EXPECT_EQ(collectMoves(controller, board, input, 0.0), (std::vector{tetris::Move::Down}));
		EXPECT_TRUE(collectMoves(controller, board, input, 0.03).empty());
		EXPECT_EQ(collectMoves(controller, board, input, 0.02), (std::vector{tetris::Move::Down}));
	}

	TEST(TetrisBoardControllerTest, resetDiscardsQueuedCommandsAndHeldRepeat) {
		// Given
		TetrisBoardController controller;
		tetris::TetrisBoard board{40, 20, tetris::BlockType::O, tetris::BlockType::O};
		ASSERT_TRUE(controller.enqueue({InputAction::Left, 0}));
		Input input;
		input.left = true;

		// When
		controller.reset();
		const auto moves = collectMoves(controller, board, input, 1.0);

		// Then
		EXPECT_EQ(std::ranges::count(moves, tetris::Move::Left), 0);
	}

	TEST(TetrisBoardControllerTest, buffersOneRotationUntilAreCompletes) {
		// Given
		TetrisBoardController controller;
		tetris::TetrisBoard board{40, 20, tetris::BlockType::O, tetris::BlockType::O};
		ASSERT_TRUE(controller.enqueue({InputAction::HardDrop, 0}));
		collectMoves(controller, board, {}, 0.0);
		collectMoves(controller, board, {}, 1.0);
		ASSERT_TRUE(controller.enqueue({InputAction::Rotate, 1}));

		// When
		const auto firstAreMoves = collectMoves(controller, board, {}, 0.2);
		const auto secondAreMoves = collectMoves(controller, board, {}, 0.2);

		// Then
		EXPECT_TRUE(firstAreMoves.empty());
		EXPECT_TRUE(secondAreMoves.empty());
		EXPECT_FALSE(controller.enqueue({InputAction::Rotate, 2}));
		collectMoves(controller, board, {}, 0.2);
		EXPECT_EQ(collectMoves(controller, board, {}, 0.0), (std::vector{tetris::Move::RotateLeft}));
	}

	TEST(TetrisBoardControllerTest, acceptsOnlyOneHardDropPerPiece) {
		// Given
		TetrisBoardController controller;
		tetris::TetrisBoard board{40, 20, tetris::BlockType::O, tetris::BlockType::O};

		// When / Then
		EXPECT_TRUE(controller.enqueue({InputAction::HardDrop, 1}));
		EXPECT_FALSE(controller.enqueue({InputAction::HardDrop, 2}));
		EXPECT_EQ(collectMoves(controller, board, {}, 0.0), (std::vector{tetris::Move::DownGround}));

		collectMoves(controller, board, {}, 1.0);
		EXPECT_FALSE(controller.enqueue({InputAction::HardDrop, 3}));
		collectMoves(controller, board, {}, 0.5);

		EXPECT_TRUE(controller.enqueue({InputAction::HardDrop, 4}));
		EXPECT_EQ(collectMoves(controller, board, {}, 0.0), (std::vector{tetris::Move::DownGround}));
	}

	TEST(TetrisBoardControllerTest, appliesSpawnInputPolicyDuringAre) {
		// Given
		TetrisBoardController controller;
		tetris::TetrisBoard board{40, 20, tetris::BlockType::T, tetris::BlockType::O};
		ASSERT_TRUE(controller.enqueue({InputAction::HardDrop, 1}));
		collectMoves(controller, board, {}, 0.0);
		collectMoves(controller, board, {}, 1.0);

		// When
		EXPECT_FALSE(controller.enqueue({InputAction::Left, 2}));
		EXPECT_TRUE(controller.enqueue({InputAction::Rotate, 3}));
		EXPECT_FALSE(controller.enqueue({InputAction::Rotate, 4}));
		EXPECT_FALSE(controller.enqueue({InputAction::SoftDrop, 5}));
		EXPECT_FALSE(controller.enqueue({InputAction::HardDrop, 6}));
		Input input;
		input.left = true;
		input.down = true;
		const auto areCompletionMoves = collectMoves(controller, board, input, 0.5);
		const auto spawnMoves = collectMoves(controller, board, input, 0.0);
		const auto heldMoves = collectMoves(controller, board, input, 0.05);

		// Then
		EXPECT_EQ(spawnMoves, (std::vector{tetris::Move::RotateLeft}));
		EXPECT_EQ(std::ranges::count(areCompletionMoves, tetris::Move::Left), 1);
		EXPECT_EQ(std::ranges::count(heldMoves, tetris::Move::Down), 1);
		EXPECT_EQ(std::ranges::count(heldMoves, tetris::Move::DownGround), 0);
	}

	TEST(TetrisBoardControllerTest, horizontalTapReleasedDuringAreDoesNotMoveNextPiece) {
		// Given
		TetrisBoardController controller;
		tetris::TetrisBoard board{40, 20, tetris::BlockType::O, tetris::BlockType::O};
		ASSERT_TRUE(controller.enqueue({InputAction::HardDrop, 1}));
		collectMoves(controller, board, {}, 0.0);
		collectMoves(controller, board, {}, 1.0);
		ASSERT_FALSE(controller.enqueue({InputAction::Left, 2}));

		// When
		collectMoves(controller, board, {}, 0.5);
		const auto moves = collectMoves(controller, board, {}, 0.0);

		// Then
		EXPECT_EQ(std::ranges::count(moves, tetris::Move::Left), 0);
	}

}
