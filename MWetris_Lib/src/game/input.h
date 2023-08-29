#ifndef MWETRIS_GAME_INPUT_H
#define MWETRIS_GAME_INPUT_H

#include <compare>

namespace mwetris::game {

	struct Input {
		bool rotate : 1;
		bool down : 1;
		bool downGround : 1;
		bool left : 1;
		bool right : 1;

		friend constexpr auto operator<=>(const Input&, const Input&) = default;
	};

}

#endif
