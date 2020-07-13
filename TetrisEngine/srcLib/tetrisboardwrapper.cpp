#include "tetrisboardwrapper.h"

#include "square.h"
#include "block.h"
#include "block.h"

#include <vector>

namespace tetris {

	BlockType randomBlockType() {
		Random random;
		constexpr std::array BlockTypes{
			BlockType::I, BlockType::J, BlockType::L,
			BlockType::O ,BlockType::S, BlockType::T, BlockType::Z};

		return BlockTypes[random.generateInt(0, static_cast<int>(BlockTypes.size()) - 1)];
	}

	std::vector<BlockType> generateRow(const TetrisBoard& board, double squaresPerLength) {
		const auto size = board.getColumns();

		Random random;
		std::vector<bool> row(size);
		for (int i = 0; i < size * squaresPerLength; ++i) {
			int index = random.generateInt(0, size - 1);
			int nbr = 0;
			while (nbr < size) {
				if (!row[(index + nbr) % size]) {
					row[(index + nbr) % size] = true;
					break;
				}
				++nbr;
			}
		}

		std::vector<BlockType> rows;
		// Fill the rows with block types.
		for (int i = 0; i < size; ++i) {
			auto blockType = BlockType::Empty;

			// Fill square?
			if (row[i]) {
				blockType = randomBlockType();
			}
			rows.push_back(blockType);
		}
		return rows;
	}

	std::vector<BlockType> generateRow(int width, int holes) {
		std::vector<BlockType> row(width);
		for (auto type : row) {
			type = randomBlockType();;
		}

		Random random;
		for (int i = 0; i < holes; ++i) {
			int index = random.generateInt(0, width - 1);
			if (row[index] == BlockType::Empty) {
				--i;
			} else {
				row[index] = BlockType::Empty;
			}
		}
		return row;
	}

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
