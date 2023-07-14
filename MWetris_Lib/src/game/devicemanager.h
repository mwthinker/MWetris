#ifndef MWETRIS_GAME_DEVICEMANAGER_H
#define MWETRIS_GAME_DEVICEMANAGER_H

#include "sdldevice.h"
#include "game/keyboard.h"
#include "game/gamepad.h"

#include <mw/signal.h>

#include <SDL.h>
#include <vector>

namespace mwetris::game {

	class DeviceManager {
	public:
		mw::PublicSignal<DeviceManager, DevicePtr> deviceFound;

		DeviceManager();

		void eventUpdate(const SDL_Event& windowEvent);

		void searchForDevice();

		void stopSearchForDevice();

		std::vector<DevicePtr> getAllDevicesAvailable() const;

		DevicePtr getDefaultDevice1() const;
		
		DevicePtr getDefaultDevice2() const;

	private:
		void controllerDeviceAddedEvent(const SDL_ControllerDeviceEvent& deviceEvent);

		void controllerDeviceRemovedEvent(const SDL_ControllerDeviceEvent& deviceEvent);

		std::shared_ptr<Keyboard> keyboard1_;
		std::shared_ptr<Keyboard> keyboard2_;
		bool searchForDevice_ = false;
		std::vector<std::shared_ptr<GamePad>> gamePads_;
	};

}

#endif
