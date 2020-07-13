#ifndef SQUARE_H
#define SQUARE_H

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
		Wall = 'W',
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
