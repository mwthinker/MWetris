#include "tetrisboard.h"

#include "square.h"
#include "block.h"
#include "block.h"

#include <vector>
#include <queue>
#include <bitset>

namespace tetris {

	BlockType randomBlockType() {
		Random random;
		const int BLOCK_TYPE_MIN = 0;
		const int BLOCK_TYPE_MAX = 6;
		static_assert((int) BlockType::Empty > BLOCK_TYPE_MAX &&
			(int) BlockType::Wall > BLOCK_TYPE_MAX, "BlockType::EMPTY should not be generated");
		// Generate a block type.
		return static_cast<BlockType>(random.generateInt(BLOCK_TYPE_MIN, BLOCK_TYPE_MAX));
	}

	std::vector<BlockType> generateRow(const RawTetrisBoard& board, double squaresPerLength) {
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
			BlockType blockType = BlockType::Empty;

			// Fill square?
			if (row[i]) {
				// Generate a block type.
				blockType = randomBlockType();
			}
			rows.push_back(blockType);
		}
		// Return the generated rows.
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

	TetrisBoard::TetrisBoard(int columns, int rows, BlockType current, BlockType next)
		: RawTetrisBoard{columns, rows, current, next} {
	}

	TetrisBoard::TetrisBoard(const std::vector<BlockType>& board,
		int columns, int rows, Block current, BlockType next,
		int savedRowsRemoved)
		: RawTetrisBoard{board, columns, rows, current, next}
		, rowsRemoved_{savedRowsRemoved} {
	}

	TetrisBoard::TetrisBoard(const TetrisBoard& board)
		: RawTetrisBoard{board}
		, turns_{board.turns_}
		, rowsRemoved_{board.rowsRemoved_} {
		
		// No copy of the listener_.
	}

	TetrisBoard& TetrisBoard::operator=(const TetrisBoard& board) {
		RawTetrisBoard::operator=(board);
		turns_ = board.turns_;
		rowsRemoved_ = board.rowsRemoved_;
		// No copy of the listener_.
		return *this;
	}

	void TetrisBoard::restart(BlockType current, BlockType next) {
		squaresToAdd_.clear();
		turns_ = 0;
		updateRestart(current, next);
	}

	void TetrisBoard::triggerEvent(BoardEvent gameEvent) {
		listener_(gameEvent, *this);
		switch (gameEvent) {
			case BoardEvent::NextBlockUpdated:
				// Assumes a new turn.
				++turns_;
				break;
			case BoardEvent::OneRowRemoved:
				rowsRemoved_ += 1;
				break;
			case BoardEvent::TwoRowRemoved:
				rowsRemoved_ += 2;
				break;
			case BoardEvent::ThreeRowRemoved:
				rowsRemoved_ += 3;
				break;
			case BoardEvent::FourRowRemoved:
				rowsRemoved_ += 4;
				break;
			case BoardEvent::Restarted:
				rowsRemoved_ = 0;
				break;
		}
	}

	void TetrisBoard::addRows(const std::vector<BlockType>& blockTypes) {
		if (!isGameOver()) {
			squaresToAdd_.insert(squaresToAdd_.end(), blockTypes.begin(), blockTypes.end());
		}
	}

	std::vector<BlockType> TetrisBoard::addExternalRows() {
		std::vector<BlockType> tmp = squaresToAdd_;
		squaresToAdd_.clear();
		return tmp;
	}

	mw::signals::Connection TetrisBoard::addGameEventListener(const std::function<void(BoardEvent, const TetrisBoard&)>& callback) {
		return listener_.connect(callback);
	}

}
