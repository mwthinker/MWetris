#ifndef MWETRIS_GAME_INPUT_H
#define MWETRIS_GAME_INPUT_H

namespace mwetris::game {

	struct Input {
		bool rotate : 1;
		bool down : 1;
		bool downGround : 1;
		bool left : 1;
		bool right : 1;
	};

	inline constexpr bool operator==(Input left, Input right) {
		return left.rotate == right.rotate
			&& left.down == right.down
			&& left.downGround == right.downGround
			&& left.left == right.left
			&& left.right == right.right;
	}

	inline constexpr bool operator!=(Input left, Input right) {
		return !(left == right);
	}

}

#endif
