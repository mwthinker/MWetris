#ifndef MWETRIS_GAME_SDLDEVICE_H
#define MWETRIS_GAME_SDLDEVICE_H

#include "device.h"

#include <SDL.h>

#include <memory>

namespace mwetris::game {

	class SdlDevice;
	typedef std::shared_ptr<SdlDevice> SdlDevicePtr;

	class SdlDevice : public Device {
	public:
		~SdlDevice() override = default;

		virtual void eventUpdate(const SDL_Event& windowEvent) {
		}

		void update(const tetris::TetrisBoard& board) override {
		}

		bool isAi() const override {
			return false;
		}
	};

}

#endif
