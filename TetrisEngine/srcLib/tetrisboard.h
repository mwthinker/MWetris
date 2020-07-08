#ifndef TETRISBOARD_H
#define TETRISBOARD_H

#include "block.h"

#include <vector>

namespace tetris {

	enum class BoardEvent {
		BlockCollision,
		CurrentBlockUpdated,
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

		void update(Move move);

		template <class EventCallback>
		void update(Move move, const std::vector<BlockType>& externalRows, EventCallback&& eventCallback);

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

	inline void TetrisBoard::update(Move move) {
		update(move, {}, [](BoardEvent boardEvent) {});
	}

	template <class EventCallback>
	void TetrisBoard::update(Move move, const std::vector<BlockType>& externalRows, EventCallback&& eventCallback) {
		if (isGameOver_ || collision(current_)) {
			if (!isGameOver_) {
				// Only called once, when the game becomes game over.
				isGameOver_ = true;
				eventCallback(BoardEvent::GameOver);
			}
			return;
		}

		Block block = current_;
		switch (move) {
			case Move::GameOver:
				// Only called once, when the game becomes game over.
				isGameOver_ = true;
				eventCallback(BoardEvent::GameOver);
				break;
			case Move::Left:
				block.moveLeft();
				if (!collision(block)) {
					current_ = block;
					eventCallback(BoardEvent::PlayerMovesBlockLeft);
				}
				break;
			case Move::Right:
				block.moveRight();
				if (!collision(block)) {
					current_ = block;
					eventCallback(BoardEvent::PlayerMovesBlockRight);
				}
				break;
			case Move::DownGround:
				eventCallback(BoardEvent::PlayerMovesBlockDownGround);
				do {
					current_ = block;
					block.moveDown();
				} while (!collision(block));
				eventCallback(BoardEvent::PlayerMovesBlockDown);
				break;
			case Move::Down:
				block.moveDown();
				if (!collision(block)) {
					current_ = block;
					eventCallback(BoardEvent::PlayerMovesBlockDown);
				}
				break;
			case Move::RotateRight:
				block.rotateRight();
				if (!collision(block)) {
					current_ = block;
					eventCallback(BoardEvent::PlayerMovesBlockUpdated);
				}
				break;
			case Move::RotateLeft:
				block.rotateLeft();
				if (!collision(block)) {
					current_ = block;
					eventCallback(BoardEvent::PlayerMovesBlockUpdated);
				}
				break;
			case Move::DownGravity:
				block.moveDown();
				if (collision(block)) {
					// Collision detected, add squares to the gameboard.
					addBlockToBoard(current_);

					eventCallback(BoardEvent::BlockCollision);

					// Remove any filled row on the gameboard.
					int nbr = removeFilledRows(current_);

					// Add rows due to some external event.
					if (!externalRows.empty()) {
						externalRowsAdded_ = static_cast<int>(externalRows.size()) / columns_;
						gameboard_.insert(gameboard_.begin(), externalRows.begin(), externalRows.end());
						eventCallback(BoardEvent::ExternalRowsAdded);
					}

					// Update the user controlled block.
					current_ = createBlock(next_);
					eventCallback(BoardEvent::CurrentBlockUpdated);

					switch (nbr) {
						case 1:
							eventCallback(BoardEvent::OneRowRemoved);
							break;
						case 2:
							eventCallback(BoardEvent::TwoRowRemoved);
							break;
						case 3:
							eventCallback(BoardEvent::ThreeRowRemoved);
							break;
						case 4:
							eventCallback(BoardEvent::FourRowRemoved);
							break;
					}
				} else {
					current_ = block;
					eventCallback(BoardEvent::GravityMovesBlock);
				}
				break;
		}
	}

}

#endif
