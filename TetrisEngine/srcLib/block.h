#ifndef BLOCK_H
#define BLOCK_H

#include <array>
#include <algorithm>

namespace tetris {

	enum class BlockType : char {
		I = 'I',
		J = 'J',
		L = 'L',
		O = 'O',
		S = 'S',
		T = 'T',
		Z = 'Z',
		Empty = 'E',
		Wall = 'W'
	};

	struct Square {
		int column;
		int row;
	};

	inline bool operator==(const Square& left, const Square& right) {
		return left.row == right.row && left.column == right.column;
	}

	inline bool operator!=(const Square& left, const Square& right) {
		return !(left != right);
	}

	class Block {
	public:
		using const_iterator = std::array<Square, 4>::const_iterator;

		Block() = default;
		Block(BlockType blockType, int startColumn, int lowestStartRow);
		Block(BlockType blockType, int startColumn, int lowestStartRow, int rotations);

		void moveLeft();
		void moveRight();
		void moveDown();
		void rotateLeft();
		void rotateRight();

		Square operator[](int index) const {
			return squares_[index];
		}

		constexpr size_t getSize() const {
			return squares_.size();
		}

		Square getRotationSquare() const {
			return squares_[rotationSquareIndex_];
		}

		BlockType getBlockType() const {
			return blockType_;
		}

		// Return the lowest row when the block is in default rotation.
		// I.e. When the constructor is called with no rotation defined.
		int getLowestStartRow() const {
			return lowestStartRow_;
		}

		int getLowestRow() const {
			return std::min({squares_[0].row, squares_[1].row, squares_[2].row, squares_[3].row});;
		}

		const_iterator begin() const {
			return squares_.begin();
		}

		const_iterator end() const {
			return squares_.end();
		}

		int getNumberOfRotations() const {
			return maxRotations_;
		}

		int getCurrentRotation() const {
			return currentRotation_;
		}

		int getStartColumn() const {
			return startColumn_;
		}

	private:
		void rotate(int rotate);

		std::array<Square, 4> squares_{};
		int rotationSquareIndex_{};
		int maxRotations_{4};
		int currentRotation_{};
		int lowestStartRow_{};
		int startColumn_{};
		BlockType blockType_{BlockType::Empty};
	};

}

#endif
