#ifndef APP_GAME_INPUT_H
#define APP_GAME_INPUT_H

#include <compare>

namespace app::game {

	struct KeyState {
		bool held = false;
		bool pressed = false;
		bool released = false;
	};

	struct Input {
		KeyState rotate;
		KeyState down;
		KeyState downGround;
		KeyState left;
		KeyState right;

		friend constexpr auto operator<=>(const Input&, const Input&) = default;
	};

}

#endif
