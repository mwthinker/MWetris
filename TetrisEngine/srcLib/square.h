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
		bool operator==(const Square& square) const = default;

		bool operator!=(const Square& square) const = default;

		int column;
		int row;
	};

} // Namespace tetris.

#endif	// SQUARE_H
