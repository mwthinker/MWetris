#include "block.h"

#include <algorithm>

namespace tetris {

	Block::Block(BlockType blockType, int startColumn, int lowestStartRow, int currentRotation)
		: Block{blockType, startColumn, lowestStartRow} {

		for (int i = 0; i < currentRotation; ++i) {
			rotateLeft();
		}
	}

	Block::Block(BlockType blockType, int startColumn, int lowestStartRow)
		: blockType_{blockType}
		, startColumn_{startColumn}
		, lowestStartRow_{lowestStartRow} {

		int nbrOfSquares = 0;
		switch (blockType) {
			case BlockType::Empty:
				maxRotations_ = 0;
				squares_[nbrOfSquares++] = Square{0, 0};
				squares_[nbrOfSquares++] = Square{0, 0};
				squares_[nbrOfSquares++] = Square{0, 0};
				squares_[nbrOfSquares++] = Square{0, 0};
				break;
			case BlockType::I:
				squares_[nbrOfSquares++] = Square{startColumn, lowestStartRow_ + 3};
				squares_[nbrOfSquares++] = Square{startColumn, lowestStartRow_ + 2};
				rotationSquareIndex_ = nbrOfSquares;
				squares_[nbrOfSquares++] = Square{startColumn, lowestStartRow_ + 1};
				squares_[nbrOfSquares++] = Square{startColumn, lowestStartRow_};
				maxRotations_ = 1;
				break;
			case BlockType::J:
				squares_[nbrOfSquares++] = Square{startColumn + 1, lowestStartRow_ + 2};
				rotationSquareIndex_ = nbrOfSquares;
				squares_[nbrOfSquares++] = Square{startColumn + 1, lowestStartRow_ + 1};
				squares_[nbrOfSquares++] = Square{startColumn + 1, lowestStartRow_};
				squares_[nbrOfSquares++] = Square{startColumn, lowestStartRow_};
				break;
			case BlockType::L:
				squares_[nbrOfSquares++] = Square{startColumn, lowestStartRow_ + 2};
				rotationSquareIndex_ = nbrOfSquares;
				squares_[nbrOfSquares++] = Square{startColumn, lowestStartRow_ + 1};
				squares_[nbrOfSquares++] = Square{startColumn, lowestStartRow_};
				squares_[nbrOfSquares++] = Square{startColumn + 1, lowestStartRow_};
				break;
			case BlockType::O:
				squares_[nbrOfSquares++] = Square{startColumn, lowestStartRow_ + 1};
				squares_[nbrOfSquares++] = Square{startColumn + 1, lowestStartRow_ + 1};
				squares_[nbrOfSquares++] = Square{startColumn, lowestStartRow_};
				squares_[nbrOfSquares++] = Square{startColumn + 1, lowestStartRow_};
				maxRotations_ = 0;
				break;
			case BlockType::S:
				squares_[nbrOfSquares++] = Square{startColumn + 1, lowestStartRow_ + 1};
				squares_[nbrOfSquares++] = Square{startColumn, lowestStartRow_ + 1};
				rotationSquareIndex_ = nbrOfSquares;
				squares_[nbrOfSquares++] = Square{startColumn, lowestStartRow_};
				squares_[nbrOfSquares++] = Square{startColumn - 1, lowestStartRow_};
				maxRotations_ = 1;
				break;
			case BlockType::T:
				squares_[nbrOfSquares++] = Square{startColumn, lowestStartRow_ + 1};
				squares_[nbrOfSquares++] = Square{startColumn + 1, lowestStartRow_};
				rotationSquareIndex_ = nbrOfSquares;
				squares_[nbrOfSquares++] = Square{startColumn, lowestStartRow_};
				squares_[nbrOfSquares++] = Square{startColumn - 1, lowestStartRow_};
				break;
			case BlockType::Z:
				squares_[nbrOfSquares++] = Square{startColumn - 1, lowestStartRow_ + 1};
				squares_[nbrOfSquares++] = Square{startColumn, lowestStartRow_ + 1};
				rotationSquareIndex_ = nbrOfSquares;
				squares_[nbrOfSquares++] = Square{startColumn, lowestStartRow_};
				squares_[nbrOfSquares++] = Square{startColumn + 1, lowestStartRow_};
				maxRotations_ = 1;
				break;
		}
	}

	void Block::moveLeft() {
		--startColumn_;
		for (auto& sq : squares_) {
			--sq.column;
		}
	}

	void Block::moveRight() {
		++startColumn_;
		for (auto& sq : squares_) {
			++sq.column;
		}
	}

	void Block::moveDown() {
		--lowestStartRow_;
		for (auto& sq : squares_) {
			--sq.row;
		}
	}

	void Block::rotate(int rotate) {
		int row = squares_[rotationSquareIndex_].row;
		int column = squares_[rotationSquareIndex_].column;

		if (rotate > 0) {
			// Rotate left.
			for (int i = 0; i < rotate; ++i) {
				for (auto& sq : squares_) {
					auto tmp = sq;
					tmp.column = column + row - sq.row;
					tmp.row = sq.column + row - column;
					sq = tmp;
				}
			}
		} else {
			rotate *= -1;
			for (int i = 0; i < rotate; ++i) {
				// Rotate right.
				for (Square& sq : squares_) {
					Square tmp = sq;
					tmp.column = sq.row + column - row;
					tmp.row = column + row - sq.column;
					sq = tmp;
				}
			}
		}
	}

	void Block::rotateLeft() {
		int row = squares_[rotationSquareIndex_].row;
		int column = squares_[rotationSquareIndex_].column;
		currentRotation_ = (currentRotation_ + 1) % 4;

		// Rotate back to start orientation?
		if (currentRotation_ > maxRotations_) {
			currentRotation_ = 0;
			// Rotate right in order to get back to default orientation.
			rotate(-maxRotations_);
		} else {
			// Rotate to the left!
			rotate(1);
		}
	}

	void Block::rotateRight() {
		int row = squares_[rotationSquareIndex_].row;
		int column = squares_[rotationSquareIndex_].column;
		currentRotation_ = (currentRotation_ + 3) % 4;

		// Rotate back to start orientation?
		if (currentRotation_ > maxRotations_) {
			currentRotation_ = 0;
			rotate(maxRotations_);
		} else {
			// Rotate to the right!
			rotate(-1);
		}
	}

}
