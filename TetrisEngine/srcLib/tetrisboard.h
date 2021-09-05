#ifndef TETRISBOARD_H
#define TETRISBOARD_H

#include "block.h"

#include <vector>
#include <type_traits>

namespace tetris {

	enum class BoardEvent {
		BlockCollision,
		CurrentBlockUpdated,
		
		PlayerMovesBlock,
		GravityMovesBlock,
		
		RowsRemoved,
		RowToBeRemoved,
		GameOver
	};

	enum class Move {
		RotateLeft,
		RotateRight,
		DownGravity,
		Down,
		DownGround,
		Left,
		Right,
		GameOver
	};

	class TetrisBoard {
	public:
		TetrisBoard(int columns, int rows, BlockType current, BlockType next);
		TetrisBoard(const std::vector<BlockType>& board,
			int columns, int rows, const Block& current, BlockType next);

		TetrisBoard(const TetrisBoard&) = default;
		TetrisBoard& operator=(const TetrisBoard&) = default;

		TetrisBoard(TetrisBoard&& other) = default;
		TetrisBoard& operator=(TetrisBoard&& other) = default;

		template <typename Rows>
		int addExternalRows(const Rows& externalRows);

		void update(Move move);

		template <typename EventCallback>
		void update(Move move, EventCallback&& eventCallback);

		void setNextBlock(BlockType next);

		void restart(BlockType current, BlockType next);

		void restart(int column, int row, BlockType current, BlockType next);

		int getRows() const {
			return rows_;
		}

		int getColumns() const {
			return columns_;
		}

		bool isGameOver() const {
			return isGameOver_;
		}

		// Return all non moving squares on the board. Index 0 to (rows+4)*columns-1.
		// All squares are saved in row major order and in ascending order.
		const std::vector<BlockType>& getBoardVector() const;

		// Return the moving block.
		Block getBlock() const {
			return current_;
		}

		// Return the type of the moving block.
		BlockType getBlockType() const {
			return current_.getBlockType();
		}

		// Return the type of the next block.
		BlockType getNextBlockType() const {
			return next_;
		}

		// Return the blocktype for a given position.
		BlockType getBlockType(int column, int row) const;

		int calculateSquaresFilled(int row) const;

		bool collision(const Block& block) const;

	private:
		void removeUnfilledRows();

		void removeEmptyRowsOutsideBoard();

		bool isRowInsideBoard(int row) const;

		BlockType& board(int column, int row) {
			return gameboard_[row * columns_ + column];
		}

		BlockType board(int column, int row) const {
			return gameboard_[row * columns_ + column];
		}

		Block createBlock(BlockType blockType) const;

		bool isRowEmpty(int row) const;

		bool isRowFilled(int row) const;

		void addBlockToBoard(const Block& block);

		template <typename EventCallback>
		void removeFilledRows(const Block& block, EventCallback& callback);

		template <typename EventCallback>
		void moveRowsOneStepDown(int rowToRemove, EventCallback& callback);

		std::vector<BlockType> gameboard_;
		BlockType next_;
		Block current_;
		int columns_;
		int rows_;
		bool isGameOver_{false};
	};

	template <typename Rows>
	int TetrisBoard::addExternalRows(const Rows& externalRows) {
		gameboard_.insert(gameboard_.begin(), std::begin(externalRows), std::end(externalRows));
		return static_cast<int>(externalRows.size()) / columns_;
	}

	inline void TetrisBoard::update(Move move) {
		update(move, [](BoardEvent boardEvent, int value) {});
	}

	template <typename EventCallback>
	void TetrisBoard::update(Move move, EventCallback&& eventCallback) {
		static_assert(std::is_invocable_v<EventCallback, BoardEvent, int>, "EventCallback must be in the form: void(BoardEvent, int) ");

		if (isGameOver_) {
			return;
		}

		if (collision(current_)) {
			isGameOver_ = true;
			eventCallback(BoardEvent::GameOver, 0);
			return;
		}

		Block block = current_;
		switch (move) {
			case Move::GameOver:
				// Only called once, when the game becomes game over.
				isGameOver_ = true;
				eventCallback(BoardEvent::GameOver, 0);
				break;
			case Move::Left:
				block.moveLeft();
				if (!collision(block)) {
					current_ = block;
					eventCallback(BoardEvent::PlayerMovesBlock, 0);
				}
				break;
			case Move::Right:
				block.moveRight();
				if (!collision(block)) {
					current_ = block;
					eventCallback(BoardEvent::PlayerMovesBlock, 0);
				}
				break;
			case Move::DownGround: {
				while (!collision(block)) {
					current_ = block;
					block.moveDown();
				}
				eventCallback(BoardEvent::PlayerMovesBlock, 0);
				break;
			}
			case Move::Down:
				block.moveDown();
				if (!collision(block)) {
					current_ = block;
					eventCallback(BoardEvent::PlayerMovesBlock, 0);
				}
				break;
			case Move::RotateRight:
				block.rotateRight();
				if (!collision(block)) {
					current_ = block;
					eventCallback(BoardEvent::PlayerMovesBlock, 0);
				}
				break;
			case Move::RotateLeft:
				block.rotateLeft();
				if (!collision(block)) {
					current_ = block;
					eventCallback(BoardEvent::PlayerMovesBlock, 0);
				}
				break;
			case Move::DownGravity:
				block.moveDown();
				if (collision(block)) {
					addBlockToBoard(current_);
					eventCallback(BoardEvent::BlockCollision, 0);
					
					removeFilledRows(current_, eventCallback);

					// Update the user controlled block.
					current_ = createBlock(next_);
					eventCallback(BoardEvent::CurrentBlockUpdated, 0);
				} else {
					current_ = block;
					eventCallback(BoardEvent::GravityMovesBlock, 0);
				}
				break;
		}
	}

	template <typename EventCallback>
	void TetrisBoard::removeFilledRows(const Block& block, EventCallback& callback) {
		int row = block.getLowestRow();
		int rowsFilled = 0;
		const int nbrOfSquares = static_cast<int>(current_.getSize());
		for (int i = 0; i < nbrOfSquares; ++i) {
			bool filled = false;
			if (isRowInsideBoard(row)) {
				filled = isRowFilled(row);
			}
			if (filled) {
				moveRowsOneStepDown(row, callback);
				++rowsFilled;
			} else {
				++row;
			}
		}

		if (rowsFilled > 0) {
			callback(BoardEvent::RowsRemoved, rowsFilled);
		}
	}

	template <typename EventCallback>
	void TetrisBoard::moveRowsOneStepDown(int rowToRemove, EventCallback& callback) {
		callback(BoardEvent::RowToBeRemoved, rowToRemove);

		auto indexStartOfRow = rowToRemove * columns_;
		gameboard_.erase(gameboard_.begin() + indexStartOfRow, gameboard_.begin() + indexStartOfRow + columns_);

		// Is it necessary to replace the row?
		if (static_cast<int>(gameboard_.size()) < rows_ * columns_) {
			// Replace the removed row with an empty row at the top.
			gameboard_.insert(gameboard_.end(), columns_, BlockType::Empty);
		}
	}

}

#endif
