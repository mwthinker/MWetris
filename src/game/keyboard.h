#ifndef MWETRIS_GAME_KEYBOARD_H
#define MWETRIS_GAME_KEYBOARD_H

#include "sdldevice.h"

#include <SDL.h>

namespace mwetris::game {

	class Keyboard : public SdlDevice {
	public:
		Keyboard(std::string name, SDL_Keycode down, SDL_Keycode left, SDL_Keycode right, SDL_Keycode rotate, SDL_Keycode downGround);

		Input getInput() const override;
		std::string getName() const override;

	private:
		void eventUpdate(const SDL_Event& windowEvent) override;

		Input input_{};
		SDL_Keycode down_{}, right_{}, left_{}, rotate_{}, downGround_{};
		std::string name_;
		std::string playerName_;
	};

}

#endif
