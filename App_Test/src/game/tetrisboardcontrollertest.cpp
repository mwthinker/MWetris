#include <gtest/gtest.h>

#include <app/game/tetrisboardcontroller.h>

#include <tetris/tetrisboard.h>

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
			key = {.held = true, .pressed = true};

			EXPECT_EQ(collectMoves(controller, board, input, 0.0), (std::vector{direction}));
			key.pressed = false;
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
		input.left = {.held = true, .pressed = true};
		input.right = {.held = true, .pressed = true};
		EXPECT_EQ(
			collectMoves(controller, board, input, 0.0),
			(std::vector{tetris::Move::Left, tetris::Move::Right}));

		input.left.pressed = false;
		input.right.pressed = false;
		EXPECT_TRUE(collectMoves(controller, board, input, 0.15).empty());
		controller.setInputTiming(0.3, 0.05);
		EXPECT_TRUE(collectMoves(controller, board, input, 0.16).empty());
		EXPECT_EQ(
			collectMoves(controller, board, input, 0.15),
			(std::vector{tetris::Move::Left, tetris::Move::Right}));
	}

}
