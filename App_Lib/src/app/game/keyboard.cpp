#include "keyboard.h"

#include <SDL.h>

#include <string>

namespace app::game {

	Keyboard::Keyboard(const std::string& name, const KeyboardMapping& keyboardMapping)
		: name_{name}
		, keyboardMapping_{keyboardMapping} {
	}

	Input Keyboard::getInput() const {
		return input_;
	}

	const char* Keyboard::getName() const {
		return name_.c_str();
	}

	bool Keyboard::eventUpdate(const SDL_Event& windowEvent) {
		SDL_Keycode key = windowEvent.key.keysym.sym;

		switch (windowEvent.type) {
			case SDL_KEYDOWN:
				if (key == keyboardMapping_.down) {
					input_.down = true;
					return true;
				} else if (key == keyboardMapping_.left) {
					input_.left = true;
					return true;
				} else if (key == keyboardMapping_.right) {
					input_.right = true;
					return true;
				} else if (key == keyboardMapping_.rotate) {
					input_.rotate = true;
					return true;
				} else if (key == keyboardMapping_.downGround) {
					input_.downGround = true;
					return true;
				}
				break;
			case SDL_KEYUP:
				if (key == keyboardMapping_.down) {
					input_.down = false;
					return true;
				} else if (key == keyboardMapping_.left) {
					input_.left = false;
					return true;
				} else if (key == keyboardMapping_.right) {
					input_.right = false;
					return true;
				} else if (key == keyboardMapping_.rotate) {
					input_.rotate = false;
					return true;
				} else if (key == keyboardMapping_.downGround) {
					input_.downGround = false;
					return true;
				}
				break;
		}
		return false;
	}

}
