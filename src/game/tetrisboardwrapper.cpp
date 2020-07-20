#include "tetrisboardwrapper.h"
#include "block.h"

#include <vector>

namespace tetris::game {

	TetrisBoardWrapper::TetrisBoardWrapper(const TetrisBoard& tetrisBoard, int savedRowsRemoved)
		: tetrisBoard_{tetrisBoard} {

		//rowsRemoved_ = savedRowsRemoved;
	}

	void TetrisBoardWrapper::addRows(const std::vector<BlockType>& blockTypes) {
		if (!tetrisBoard_.isGameOver()) {
			squaresToAdd_.insert(squaresToAdd_.begin(), blockTypes.begin(), blockTypes.end());
		}
	}

	void TetrisBoardWrapper::restart(BlockType current, BlockType next) {
		squaresToAdd_.clear();
		turns_ = 0;
		nbrOneLines_ = 0;
		nbrTwoLines_ = 0;
		nbrThreeLines_ = 0;
		nbrFourLines_ = 0;
		tetrisBoard_.restart(current, next);
	}

	const TetrisBoard& TetrisBoardWrapper::getTetrisBoard() const {
		return tetrisBoard_;
	}

}
