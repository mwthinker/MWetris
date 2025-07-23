#ifndef APP_GAME_DEVICEMANAGER_H
#define APP_GAME_DEVICEMANAGER_H

#include "keyboard.h"
#include "gamepad.h"
#include "device.h"

#include <mw/signal.h>

#include <SDL3/SDL.h>
#include <vector>

namespace app::game {

	class DeviceManager : public std::enable_shared_from_this<DeviceManager> {
	public:
		mw::PublicSignal<DeviceManager, DevicePtr> deviceConnected;

		DeviceManager();

		void eventUpdate(const SDL_Event& windowEvent);

		// Called every frame to update the state of devices
		void tick();

		std::vector<DevicePtr> getAllDevicesAvailable() const;

		DevicePtr getDefaultDevice1() const;
		
		DevicePtr getDefaultDevice2() const;

		DevicePtr findDevice(const std::string& guid) const;

	private:
		struct KeyboardDevice {
			std::unique_ptr<Keyboard> keyboard;
			DevicePtr device;
		};
		
		static KeyboardDevice createKeyboardDevice(std::unique_ptr<Keyboard> keyboard);

		void controllerDeviceAddedEvent(const SDL_GamepadDeviceEvent& deviceEvent);

		void controllerDeviceRemovedEvent(const SDL_GamepadDeviceEvent& deviceEvent);
		
		KeyboardDevice keyboard1_;
		KeyboardDevice keyboard2_;

		struct Pair {
			std::unique_ptr<GamePad> gamePad;
			DevicePtr device;
			SDL_GUID guid;
		};
		std::vector<Pair> devices_;
	};

}

#endif
