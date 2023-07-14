#include "computer.h"
#include "tetrisboard.h"

#include <vector>
#include <string>

namespace mwetris::game {

	namespace {

		// Calculate and return the best input to achieve the current state.
		Input calculateInput(tetris::Ai::State state, Input current) {
			Input input{};
			if (state.rotationLeft > 0) {
				input.rotate = !current.rotate;
			}
			if (state.left > 0) {
				input.left = true;
			} else if (state.left < 0) {
				input.right = true;
			}
			if (state.left == 0 && state.rotationLeft == 0) {
				input.down = true;
			}
			return input;
		}

	}

	Computer::Computer(const tetris::Ai& ai)
		: ai_{ai} {
	}

	Input Computer::getInput() const {
		return input_;
	}

	const char* Computer::getName() const {
		return ai_.getName().c_str();
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

		input_ = calculateInput(state_, input_);
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
