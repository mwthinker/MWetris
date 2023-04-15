#ifndef MWETRIS_GAME_SDLDEVICE_H
#define MWETRIS_GAME_SDLDEVICE_H

#include "device.h"

#include <SDL.h>

#include <memory>

namespace mwetris::game {

	class SdlDevice;
	using SdlDevicePtr = std::shared_ptr<SdlDevice>;

	class SdlDevice : public Device {
	public:
		~SdlDevice() override = default;

		virtual void eventUpdate(const SDL_Event& windowEvent) = 0;
	};

}

#endif
