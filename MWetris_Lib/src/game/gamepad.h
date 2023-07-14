#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include "sdldevice.h"

#include <sdl/gamecontroller.h>

namespace mwetris::game {

	class GamePad : public SdlDevice {
	public:
		GamePad(sdl::GameController&& gameController, int rotateButton = 0, int downButton = 1);

		Input getInput() const override;
		const char* getName() const override;

		int getInstanceId() const;

		const std::string& getGuid() const;

		bool eventUpdate(const SDL_Event& windowEvent) override;

	private:
		bool updateInput(Uint8 button, bool state);

		Input input_{};
		sdl::GameController gameController_;
		int rotateButton_, downButton_;
		std::string playerName_;
		mutable std::string guid_; // Cache the value
	};

}

#endif
