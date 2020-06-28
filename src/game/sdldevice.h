#ifndef MWETRIS_GAME_SDLDEVICE_H
#define MWETRIS_GAME_SDLDEVICE_H

#include "device.h"

#include <SDL.h>

#include <memory>

namespace tetris::game {

	class SdlDevice;
	typedef std::shared_ptr<SdlDevice> SdlDevicePtr;

	class SdlDevice : public Device {
	public:
		virtual ~SdlDevice() = default;

		virtual void eventUpdate(const SDL_Event& windowEvent) {
		}

		void update(const TetrisBoard& board) override {
		}

		bool isAi() const override {
			return false;
		}
	};

}

#endif
