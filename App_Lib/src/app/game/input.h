#ifndef APP_GAME_INPUT_H
#define APP_GAME_INPUT_H

#include "inputbindings.h"

#include <cstdint>

namespace app::game {

	enum class InputState {
		Pressed,
		Released,
	};

	struct InputEvent {
		InputAction action = InputAction::Left;
		InputState state = InputState::Released;
		std::uint64_t timestamp = 0;

		friend constexpr bool operator==(const InputEvent&, const InputEvent&) = default;
	};

	struct InputCommand {
		InputAction action = InputAction::Left;
		std::uint64_t timestamp = 0;
		bool deferredUntilNextPiece = false;

		friend constexpr bool operator==(const InputCommand&, const InputCommand&) = default;
	};

	struct Input {
		bool rotate = false;
		bool down = false;
		bool downGround = false;
		bool left = false;
		bool right = false;

		friend constexpr bool operator==(const Input&, const Input&) = default;
	};

}

#endif
