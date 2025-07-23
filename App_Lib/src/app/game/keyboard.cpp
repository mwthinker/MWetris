#include "keyboard.h"

#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

#include <string>

namespace app::game {

	namespace {

		void handleKey(KeyState& keyState, bool isDown) {
			if (isDown) {
				if (!keyState.held) {
					keyState.pressed = true; // Trigger only on key-down transition
				}
				keyState.held = true;
			} else {
				if (keyState.held) {
					keyState.released = true; // Trigger only on key-up transition
				}
				keyState.held = false;
			}
		}

	}

	void Keyboard::tick() {
		input_.left.pressed = false;
		input_.left.released = false;

		input_.right.pressed = false;
		input_.right.released = false;

		input_.down.pressed = false;
		input_.down.released = false;

		input_.rotate.pressed = false;
		input_.rotate.released = false;

		input_.downGround.pressed = false;
		input_.downGround.released = false;
	}

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
		if (windowEvent.type != SDL_EVENT_KEY_DOWN && windowEvent.type != SDL_EVENT_KEY_UP) {
			return false;
		}

		if (windowEvent.key.repeat != 0) {
			return false;
		}

		bool isDown = (windowEvent.type == SDL_EVENT_KEY_DOWN);
		SDL_Keycode key = windowEvent.key.key;

		if (key == keyboardMapping_.left) {
			handleKey(input_.left, isDown);
		} else if (key == keyboardMapping_.right) {
			handleKey(input_.right, isDown);
		} else if (key == keyboardMapping_.down) {
			handleKey(input_.down, isDown);
		} else if (key == keyboardMapping_.rotate) {
			handleKey(input_.rotate, isDown);
		} else if (key == keyboardMapping_.downGround) {
			handleKey(input_.downGround, isDown);
		}
		return true;
	}

}
