#ifndef RAWTETRISBOARD_H
#define RAWTETRISBOARD_H

#include "block.h"

#include <vector>

namespace tetris {

	enum class BoardEvent {
		Restarted,
		BlockCollision,
		CurrentBlockUpdated,
		NextBlockUpdated,
		ExternalRowsAdded,
		
		PlayerMovesBlockUpdated,
		PlayerMovesBlockLeft,
		PlayerMovesBlockRight,
		PlayerMovesBlockDown,
		PlayerMovesBlockDownGround,
		GravityMovesBlock,
		
		OneRowRemoved,
		TwoRowRemoved,
		ThreeRowRemoved,
		FourRowRemoved,
		RowToBeRemoved,
		GameOver,
		
		MoveRotateLeft,
		MoveRotateRight,
		MoveDownGravity,
		MoveDown,
		MoveDownGround,
		MoveLeft,
		MoveRigh,
		MoveGameOver
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

	class RawTetrisBoard {
	public:
		RawTetrisBoard(int columns, int rows, BlockType current, BlockType next);
		RawTetrisBoard(const std::vector<BlockType>& board,
			int columns, int rows, const Block& current, BlockType next);

		RawTetrisBoard(const RawTetrisBoard&) = default;
		RawTetrisBoard& operator=(const RawTetrisBoard&) = default;

		RawTetrisBoard(RawTetrisBoard&& other) = default;
		RawTetrisBoard& operator=(RawTetrisBoard&& other) = default;

		virtual ~RawTetrisBoard() = default;

		void update(Move move);

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

		// Return true if the block is outside or on an already occupied square on the board.
		// Otherwise it return false.
		bool collision(const Block& block) const;

		int getNbrExternalRowsAdded() const {
			return externalRowsAdded_;
		}

		int getRowToBeRemoved() const {
			return rowToBeRemoved_;
		}

	private:
		bool isRowInsideBoard(int row) const;

		BlockType& board(int column, int row) {
			return gameboard_[row * columns_ + column];
		}

		BlockType board(int column, int row) const {
			return gameboard_[row * columns_ + column];
		}

		Block createBlock(BlockType blockType) const;

		bool isRowEmpty(int row) const {
			for (int column = 0; column < columns_; ++column) {
				if (board(column, row) != BlockType::Empty) {
					return false;
				}
			}
			return true;
		}

		bool isRowFilled(int row) const {
			for (int column = 0; column < columns_; ++column) {
				if (board(column, row) == BlockType::Empty) {
					return false;
				}
			}
			return true;
		}

		// Set all squares on the board to empty.
		// Game over is set to false.
		void clearBoard();

		// Is called just after the event given is triggered.
		// e.g. if the row removed event is triggered (this function is called), the board has
		// already removed the row mentioned.
		virtual void triggerEvent(BoardEvent gameEvent) {
		}

		virtual std::vector<BlockType> addExternalRows() {
			return std::vector<BlockType>(0);
		}

		void addBlockToBoard(const Block& block);

		int removeFilledRows(const Block& block);
		void moveRowsOneStepDown(int rowToRemove);

		std::vector<BlockType> gameboard_;	// Containing all non moving squares on the board.
		BlockType next_;					// Next block for the player to control.
		Block current_;						// The current block for the player to control.
		int columns_;
		int rows_;
		bool isGameOver_{false};
		int externalRowsAdded_{0};
		int rowToBeRemoved_{-1};
	};

}

#endif
