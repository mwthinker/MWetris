#include "computer.h"
#include "tetrisboardwrapper.h"

#include <vector>
#include <string>
#include <future>

namespace mwetris::game {

	Computer::Computer(const tetris::Ai& ai)
		: ai_{ai} {
	}

	Input Computer::getInput() const {
		return input_;
	}

	std::string Computer::getName() const {
		return ai_.getName();
	}

	void Computer::update(const TetrisBoardWrapper& board) {
		// New block appears?
		if (currentTurn_ != board.getTurns() && !activeThread_) {
			activeThread_ = true;
			input_ = {};
			currentTurn_ = board.getTurns();

			handle_ = std::async(std::launch::async | std::launch::deferred, Computer::calculateBestState, board.getTetrisBoard(), ai_, 1);
		} else {
			if (handle_.valid()) {
				latestState_ = handle_.get();
				latestBlock_ = board.getBlock();
				handle_ = std::future<tetris::Ai::State>();
				activeThread_ = false;
			}
			auto current = board.getBlock();
			auto currentSq = current.getRotationSquare();
			auto sq = latestBlock_.getRotationSquare();

			if (latestState_.left == sq.column - currentSq.column) {
				latestState_.left = 0;
			}

			if (latestState_.rotationLeft == current.getCurrentRotation() - latestBlock_.getCurrentRotation()) {
				latestState_.rotationLeft = 0;
			}

			input_ = calculateInput(latestState_);
		}
	}

	tetris::Ai::State Computer::calculateBestState(tetris::TetrisBoard board, tetris::Ai ai, int depth) {
		return ai.calculateBestState(board, depth);
	}

	// Calculate and return the best input to achieve the current state.
	Input Computer::calculateInput(tetris::Ai::State state) const {
		Input input{};
		if (state.rotationLeft > 0) {
			input.rotate = !input_.rotate;
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
