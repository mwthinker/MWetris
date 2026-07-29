#ifndef APP_GAME_DEVICEMANAGER_H
#define APP_GAME_DEVICEMANAGER_H

#include "keyboard.h"
#include "gamepad.h"
#include "device.h"
#include "../controlprofiles.h"

#include <mw/signal.h>

#include <SDL3/SDL.h>
#include <vector>

namespace app::game {

	class DeviceManager : public std::enable_shared_from_this<DeviceManager> {
	public:
		mw::PublicSignal<DeviceManager, DevicePtr> deviceConnected;

		explicit DeviceManager(std::shared_ptr<ControlProfiles> controlProfiles);

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
		
		KeyboardDevice createKeyboardDevice(std::unique_ptr<Keyboard> keyboard);
		void refreshInputTiming(const DevicePtr& device);
		void onControlProfilesChanged(const ControlProfilesChange& change);

		void controllerDeviceAddedEvent(const SDL_GamepadDeviceEvent& deviceEvent);

		void controllerDeviceRemovedEvent(const SDL_GamepadDeviceEvent& deviceEvent);
		
		std::shared_ptr<ControlProfiles> controlProfiles_;
		mw::signals::ScopedConnections connections_;

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
