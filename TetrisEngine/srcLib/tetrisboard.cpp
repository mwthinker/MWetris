#include "tetrisboard.h"
#include "square.h"
#include "block.h"

#include <cassert>

namespace tetris {

	TetrisBoard::TetrisBoard(int columns, int rows, BlockType current, BlockType next)
		: gameboard_{rows * columns, BlockType::Empty}
		, next_{next}
		, columns_{columns}
		, rows_{rows} {

		// Uses the size of the board. I.e. rows_ and columns_.
		current_ = createBlock(current);
	}

	TetrisBoard::TetrisBoard(const std::vector<BlockType>& board, int columns, int rows, const Block& current, BlockType next)
		: TetrisBoard{columns, rows, current.getBlockType(), next} {

		gameboard_.insert(gameboard_.begin(), board.begin(), board.end());

		int calcRows = static_cast<int>(gameboard_.size()) / columns_; // Number of whole rows.
		int nbr = static_cast<int>(gameboard_.size()) - calcRows * columns_; // The number of elements in the unfilled row.

		// To make all rows filled. Remove elements in the unfilled row.
		for (int i = 0; i < nbr; ++i) {
			gameboard_.pop_back();
		}

		// Remove unneeded rows. I.e. remove empty rows at the top which are outside the board.
		for (int row = calcRows - 1; row >= rows_; --row) {
			if (isRowEmpty(row)) {
				for (int column = 0; column < columns_; ++column) {
					gameboard_.pop_back();
				}
			}
		}

		if (collision(current)) {
			isGameOver_ = true;
		}
		current_ = current;
	}

	void TetrisBoard::setNextBlock(BlockType nextBlock) {
		next_ = nextBlock;
	}

	void TetrisBoard::restart(BlockType current, BlockType next) {
		restart(columns_, rows_, current, next);
	}

	void TetrisBoard::restart(int columns, int rows, BlockType current, BlockType next) {
		next_ = next;
		rows_ = rows;
		columns_ = columns;
		current_ = createBlock(current);
		rowToBeRemoved_ = -1;
		externalRowsAdded_ = 0;
		clearBoard();
	}

	const std::vector<BlockType>& TetrisBoard::getBoardVector() const {
		return gameboard_;
	}

	void TetrisBoard::addBlockToBoard(const Block& block) {
		for (const auto& sq : block) {
			board(sq.column, sq.row) = block.getBlockType();
		}
	}

	Block TetrisBoard::createBlock(BlockType blockType) const {
		return Block{blockType, columns_ / 2 - 1, rows_ - 4}; // 4 rows are the starting area.
	}

	bool TetrisBoard::isRowEmpty(int row) const {
		for (int column = 0; column < columns_; ++column) {
			if (board(column, row) != BlockType::Empty) {
				return false;
			}
		}
		return true;
	}

	bool TetrisBoard::isRowFilled(int row) const {
		for (int column = 0; column < columns_; ++column) {
			if (board(column, row) == BlockType::Empty) {
				return false;
			}
		}
		return true;
	}

	BlockType TetrisBoard::getBlockType(int column, int row) const {
		if (column < 0 || column >= columns_ || row < 0) {
			return BlockType::Wall;
		}
		if (row * columns_ + column >= static_cast<int>(gameboard_.size())) {
			return BlockType::Empty;
		}
		return board(column, row);
	}

	int TetrisBoard::calculateSquaresFilled(int row) const {
		if (row >= static_cast<int>(gameboard_.size()) / columns_) {
			return 0;
		}
		int filled = 0;
		for (int x = 0; x < columns_; ++x) {
			if (board(x, row) != BlockType::Empty) {
				++filled;
			}
		}
		return filled;
	}

	bool TetrisBoard::collision(const Block& block) const {
		bool collision = false;

		for (const Square& sq : block) {
			if (getBlockType(sq.column, sq.row) != BlockType::Empty) {
				collision = true;
				break;
			}
		}

		return collision;
	}

	void TetrisBoard::clearBoard() {
		gameboard_.assign(rows_ * columns_, BlockType::Empty);
		isGameOver_ = false;
	}

	bool TetrisBoard::isRowInsideBoard(int row) const {
		return row >= 0 && row * columns_ < static_cast<int>(gameboard_.size());
	}

}
