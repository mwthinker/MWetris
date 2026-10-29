#include "computer.h"

#include <tetris/tetrisboard.h>

#include <string>
#include <vector>

namespace app::game {

	namespace {

		// Calculate and return the best input to achieve the current state.
		Input calculateInput(tetris::Ai::State state) {
			Input next = {};

			// Determine target key states
			bool wantRotate = state.rotationLeft > 0;
			bool wantLeft = state.left > 0;
			bool wantRight = state.left < 0;
			bool wantDown = state.left == 0 && state.rotationLeft == 0;

			next.rotate = wantRotate;
			next.left = wantLeft;
			next.right = wantRight;
			next.down = wantDown;

			// Keep downGround unchanged for now (or add logic if needed)
			next.downGround = false;
			
			return next;
		}

	}

	Computer::Computer(const tetris::Ai& ai)
		: ai_{ai} {
	}

	Input Computer::getInput() const {
		return input_;
	}

	void Computer::onGameboardEvent(const tetris::TetrisBoard& board, tetris::BoardEvent event, int value) {
		if (event == tetris::BoardEvent::CurrentBlockUpdated) {
			state_ = ai_.calculateBestState(board, 1);
			block_ = board.getBlock();
		}

		if (isHorizontalMoveDone(board)) {
			state_.left = 0;
		}

		if (isRotationDone(board)) {
			state_.rotationLeft = 0;
		}

		input_ = calculateInput(state_);
	}

	bool Computer::isHorizontalMoveDone(const tetris::TetrisBoard& board) const {
		auto current = board.getBlock();
		auto currentSq = current.getRotationSquare();
		auto sq = block_.getRotationSquare();
		return state_.left == sq.column - currentSq.column;
	}

	bool Computer::isRotationDone(const tetris::TetrisBoard& board) const {
		auto current = board.getBlock();
		return state_.rotationLeft == current.getCurrentRotation() - block_.getCurrentRotation();
	}

}
