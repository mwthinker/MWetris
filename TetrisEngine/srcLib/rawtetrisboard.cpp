#include "rawtetrisboard.h"
#include "square.h"
#include "block.h"

#include <cassert>

namespace tetris {

	namespace {

		constexpr inline BoardEvent gameEventToMove(Move move) {
			switch (move) {
				case Move::RotateLeft:
					return BoardEvent::MoveRotateLeft;
				case Move::RotateRight:
					return BoardEvent::MoveRotateRight;
				case Move::DownGravity:
					return BoardEvent::MoveDownGravity;
				case Move::Down:
					return BoardEvent::MoveDown;
				case Move::DownGround:
					return BoardEvent::MoveDownGround;
				case Move::Left:
					return BoardEvent::MoveLeft;
				case Move::Right:
					return BoardEvent::MoveRigh;
				default:
					return BoardEvent::GameOver;
			}
		}
	}

	RawTetrisBoard::RawTetrisBoard(int columns, int rows, BlockType current, BlockType next)
		: gameboard_{rows * columns, BlockType::Empty}
		, next_{next}
		, columns_{columns}
		, rows_{rows} {

		// Uses the size of the board. I.e. rows_ and columns_.
		current_ = createBlock(current);
	}

	RawTetrisBoard::RawTetrisBoard(const std::vector<BlockType>& board, int columns, int rows, const Block& current, BlockType next)
		: RawTetrisBoard{columns, rows, current.getBlockType(), next} {

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

	void RawTetrisBoard::update(Move move) {
		triggerEvent(gameEventToMove(move));
		
		if (isGameOver_ || collision(current_)) {
			if (!isGameOver_) {
				// Only called once, when the game becomes game over.
				isGameOver_ = true;
				triggerEvent(BoardEvent::GameOver);
			}
			return;
		}

		Block block = current_;
		switch (move) {
			case Move::GameOver:
				// Only called once, when the game becomes game over.
				isGameOver_ = true;
				triggerEvent(BoardEvent::GameOver);
				break;
			case Move::Left:
				block.moveLeft();
				if (!collision(block)) {
					current_ = block;
					triggerEvent(BoardEvent::PlayerMovesBlockLeft);
				}
				break;
			case Move::Right:
				block.moveRight();
				if (!collision(block)) {
					current_ = block;
					triggerEvent(BoardEvent::PlayerMovesBlockRight);
				}
				break;
			case Move::DownGround:
				triggerEvent(BoardEvent::PlayerMovesBlockDownGround);
				do {
					current_ = block;
					block.moveDown();
				} while (!collision(block));
				triggerEvent(BoardEvent::PlayerMovesBlockDown);
				break;
			case Move::Down:
				block.moveDown();
				if (!collision(block)) {
					current_ = block;
					triggerEvent(BoardEvent::PlayerMovesBlockDown);
				}
				break;
			case Move::RotateRight:
				block.rotateRight();
				if (!collision(block)) {
					current_ = block;
					triggerEvent(BoardEvent::PlayerMovesBlockUpdated);
				}
				break;
			case Move::RotateLeft:
				block.rotateLeft();
				if (!collision(block)) {
					current_ = block;
					triggerEvent(BoardEvent::PlayerMovesBlockUpdated);
				}
				break;
			case Move::DownGravity:
				block.moveDown();
				if (collision(block)) {
					// Collision detected, add squares to the gameboard.
					addBlockToBoard(current_);

					triggerEvent(BoardEvent::BlockCollision);

					// Remove any filled row on the gameboard.
					int nbr = removeFilledRows(current_);

					// Add rows due to some external event.
					std::vector<BlockType> squares = addExternalRows();
					if (squares.size() > 0) {
						externalRowsAdded_ = static_cast<int>(squares.size()) / columns_;
						gameboard_.insert(gameboard_.begin(), squares.begin(), squares.end());
						triggerEvent(BoardEvent::ExternalRowsAdded);
					}

					// Update the user controlled block.
					current_ = createBlock(next_);
					triggerEvent(BoardEvent::CurrentBlockUpdated);

					switch (nbr) {
						case 1:
							triggerEvent(BoardEvent::OneRowRemoved);
							break;
						case 2:
							triggerEvent(BoardEvent::TwoRowRemoved);
							break;
						case 3:
							triggerEvent(BoardEvent::ThreeRowRemoved);
							break;
						case 4:
							triggerEvent(BoardEvent::FourRowRemoved);
							break;
					}
				} else {
					current_ = block;
					triggerEvent(BoardEvent::GravityMovesBlock);
				}
				break;
		}
	}

	void RawTetrisBoard::setNextBlock(BlockType nextBlock) {
		next_ = nextBlock;
		triggerEvent(BoardEvent::NextBlockUpdated);
	}

	void RawTetrisBoard::restart(BlockType current, BlockType next) {
		restart(columns_, rows_, current, next);
	}

	void RawTetrisBoard::restart(int columns, int rows, BlockType current, BlockType next) {
		next_ = next;
		rows_ = rows;
		columns_ = columns;
		current_ = createBlock(current);
		rowToBeRemoved_ = -1;
		externalRowsAdded_ = 0;
		clearBoard();
		triggerEvent(BoardEvent::Restarted);
	}

	const std::vector<BlockType>& RawTetrisBoard::getBoardVector() const {
		return gameboard_;
	}

	void RawTetrisBoard::addBlockToBoard(const Block& block) {
		// All squares in the block is added to the gameboard.
		for (const auto& sq : block) {
			board(sq.column, sq.row) = block.getBlockType();
		}
	}

	Block RawTetrisBoard::createBlock(BlockType blockType) const {
		return Block(blockType, columns_ / 2 - 1, rows_ - 4); // 4 rows are the starting area.
	}

	BlockType RawTetrisBoard::getBlockType(int column, int row) const {
		if (column < 0 || column >= columns_ || row < 0) {
			return BlockType::Wall;
		}
		if (row * columns_ + column >= (int) gameboard_.size()) {
			return BlockType::Empty;
		}
		return board(column, row);
	}

	int RawTetrisBoard::calculateSquaresFilled(int row) const {
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

	bool RawTetrisBoard::collision(const Block& block) const {
		bool collision = false;

		for (const Square& sq : block) {
			if (getBlockType(sq.column, sq.row) != BlockType::Empty) {
				collision = true;
				break;
			}
		}

		return collision;
	}

	void RawTetrisBoard::clearBoard() {
		gameboard_.assign(rows_ * columns_, BlockType::Empty);
		isGameOver_ = false;
	}

	bool RawTetrisBoard::isRowInsideBoard(int row) const {
		return row >= 0 && row * columns_ < static_cast<int>(gameboard_.size());
	}

	int RawTetrisBoard::removeFilledRows(const Block& block) {
		int row = block.getLowestRow();
		int rowsFilled = 0;
		const int nbrOfSquares = static_cast<int>(current_.getSize());
		for (int i = 0; i < nbrOfSquares; ++i) {
			bool filled = false;
			if (isRowInsideBoard(row)) {
				filled = isRowFilled(row);
			}
			if (filled) {
				moveRowsOneStepDown(row);
				++rowsFilled;
			} else {
				++row;
			}
		}

		return rowsFilled;
	}

	void RawTetrisBoard::moveRowsOneStepDown(int rowToRemove) {
		rowToBeRemoved_ = rowToRemove;
		triggerEvent(BoardEvent::RowToBeRemoved);
		
		int indexStartOfRow = rowToRemove * columns_;
		gameboard_.erase(gameboard_.begin() + indexStartOfRow, gameboard_.begin() + indexStartOfRow + columns_);

		// Is it necessary to replace the row?
		if ((int) gameboard_.size() < rows_ * columns_) {
			// Replace the removed row with an empty row at the top.
			gameboard_.insert(gameboard_.end(), columns_, BlockType::Empty);
		}
	}

}
