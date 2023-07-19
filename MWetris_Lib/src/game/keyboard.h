#ifndef MWETRIS_GAME_KEYBOARD_H
#define MWETRIS_GAME_KEYBOARD_H

#include "input.h"

#include <SDL.h>

#include <string>

namespace mwetris::game {

	struct KeyboardMapping {
		SDL_Keycode down, right, left, rotate, downGround;
	};

	class Keyboard {
	public:
		Keyboard(const std::string& name, const KeyboardMapping& keyboardMapping);

		Input getInput() const;
		const char* getName() const;

		bool eventUpdate(const SDL_Event& windowEvent);

	private:
		Input input_{};
		std::string name_;
		KeyboardMapping keyboardMapping_;
	};

}

#endif
