#include "tetrisboardwrapper.h"

#include "square.h"
#include "block.h"

#include <vector>

namespace tetris {

	void TetrisBoardWrapper::triggerEvent(BoardEvent gameEvent, int value) {
		switch (gameEvent) {
			case BoardEvent::RowToBeRemoved:
				rowToBeRemoved_ = value;
				break;
			case BoardEvent::CurrentBlockUpdated:
				externalRowsAdded_ = tetrisBoard_.addExternalRows(squaresToAdd_);
				squaresToAdd_.clear();
				++turns_;
				break;
			case BoardEvent::RowsRemoved:
				nbrOneLines_ += value;
				break;
		}
		boardEventCallbacks_(gameEvent, *this);
	}

	TetrisBoardWrapper::TetrisBoardWrapper(int columns, int rows, BlockType current, BlockType next)
		: tetrisBoard_{columns, rows, current, next} {
	}

	TetrisBoardWrapper::TetrisBoardWrapper(const std::vector<BlockType>& board,
		int columns, int rows, Block current, BlockType next,
		int savedRowsRemoved)
		: tetrisBoard_{board, columns, rows, current, next} {

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

	mw::signals::Connection TetrisBoardWrapper::addGameEventListener(const std::function<void(BoardEvent, const TetrisBoardWrapper&)>& callback) {
		return boardEventCallbacks_.connect(callback);
	}

	const TetrisBoard& TetrisBoardWrapper::getTetrisBoard() const {
		return tetrisBoard_;
	}

}
