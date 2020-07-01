#ifndef SQUARE_H
#define SQUARE_H

namespace tetris {

	enum class BlockType {
		I,
		J,
		L,
		O,
		S,
		T,
		Z,
		EMPTY,
		WALL
	};

	struct Square {
		bool operator==(const Square& square) const {
			return row == square.row && column == square.column;
		}

		bool operator!=(const Square& square) const {
			return row != square.row || column != square.column;
		}

		int column;
		int row;
	};

}

#endif
