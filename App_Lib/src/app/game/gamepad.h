#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include "input.h"

#include <sdl/gamecontroller.h>

#include <SDL3/SDL_events.h>

#include <string>

namespace app::game {

	class GamePad {
	public:
		GamePad(sdl::GameController&& gameController, int rotateButton = 0, int downButton = 1);

		Input getInput() const;
		const char* getName() const;

		int getInstanceId() const;

		const std::string& getGuid() const;

		bool eventUpdate(const SDL_Event& windowEvent);

		bool isActive() const {
			return true;
		}

		void setGameController(sdl::GameController&& gameController);

	private:
		bool updateInput(Uint8 button, bool state);

		Input input_{};
		sdl::GameController gameController_;
		int rotateButton_, downButton_;
		mutable std::string guid_; // Cache the value
	};

}

#endif
