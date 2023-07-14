#include "keyboard.h"

#include <SDL.h>

#include <string>

namespace mwetris::game {

	Keyboard::Keyboard(std::string name, SDL_Keycode down, SDL_Keycode left,
		SDL_Keycode right, SDL_Keycode rotate, SDL_Keycode downGround)
		: down_{down}
		, right_{right}
		, left_{left}
		, rotate_{rotate}
		, downGround_{downGround}
		, name_{name} {
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
				if (key == down_) {
					return input_.down = true;
				} else if (key == left_) {
					return input_.left = true;
				} else if (key == right_) {
					return input_.right = true;
				} else if (key == rotate_) {
					return input_.rotate = true;
				} else if (key == downGround_) {
					return input_.downGround = true;
				}
				break;
			case SDL_KEYUP:
				if (key == down_) {
					input_.down = false;
				} else if (key == left_) {
					input_.left = false;
				} else if (key == right_) {
					input_.right = false;
				} else if (key == rotate_) {
					input_.rotate = false;
				} else if (key == downGround_) {
					input_.downGround = false;
				}
				break;
		}
		return false;
	}

}
