#include "computer.h"
#include "tetrisboardcomponent.h"

#include <vector>
#include <string>
#include <future>

namespace tetris::game {

	Computer::Computer(const Ai& ai)
		: ai_{ai} {
	}

	Input Computer::getInput() const {
		return input_;
	}

	std::string Computer::getName() const {
		return ai_.getName();
	}

	void Computer::update(const TetrisBoardComponent& board) {
		// New block appears?
		if (currentTurn_ != board.getTurns() && !activeThread_) {
			activeThread_ = true;
			input_ = {};
			currentTurn_ = board.getTurns();

			handle_ = std::async(std::launch::async | std::launch::deferred, Computer::calculateBestState, RawTetrisBoard{board}, ai_, 1);
		} else {
			if (handle_.valid()) {
				latestState_ = handle_.get();
				latestBlock_ = board.getBlock();
				handle_ = std::future<Ai::State>();
				activeThread_ = false;
			}
			Block current = board.getBlock();
			Square currentSq = current.getRotationSquare();
			Square sq = latestBlock_.getRotationSquare();

			if (latestState_.left == sq.column - currentSq.column) {
				latestState_.left = 0;
			}

			if (latestState_.rotationLeft == current.getCurrentRotation() - latestBlock_.getCurrentRotation()) {
				latestState_.rotationLeft = 0;
			}

			input_ = calculateInput(latestState_);
		}
	}

	Ai::State Computer::calculateBestState(RawTetrisBoard board, Ai ai, int depth) {
		return ai.calculateBestState(board, depth);
	}

	// Calculate and return the best input to achieve the current state.
	Input Computer::calculateInput(Ai::State state) const {
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
