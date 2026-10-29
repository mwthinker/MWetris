#ifndef APP_GAME_DEVICEMANAGER_H
#define APP_GAME_DEVICEMANAGER_H

#include "keyboard.h"
#include "gamepad.h"
#include "device.h"
#include "inputbindings.h"

#include <mw/signal.h>

#include <SDL3/SDL.h>

#include <array>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

namespace app::game {

	class DeviceManager {
	public:
		using PersistenceCallback = std::function<DeviceBindingsPersistenceResult(const std::vector<DeviceBinding>&)>;

		mw::PublicSignal<DeviceManager, DevicePtr> deviceConnected;

		explicit DeviceManager(std::vector<DeviceBinding> bindings = {}, PersistenceCallback persist = {});

		bool eventUpdate(const SDL_Event& windowEvent);
		void tick();
		void clearInput();

		std::vector<DevicePtr> getAllDevicesAvailable() const;
		DevicePtr getDefaultDevice1() const;
		DevicePtr getDefaultDevice2() const;
		DevicePtr findDevice(const std::string& guid) const;
		void beginMapping(Device& device);
		void resetMapping(Device& device);
		void cancelMapping(Device& device);
		std::optional<MappingStatus> getMappingStatus(Device& device) const;
		std::array<std::string, 5> getMappingLabels(Device& device) const;
		DeviceBindingsPersistenceResult saveMapping(Device& device);

	private:
		struct KeyboardDevice {
			std::unique_ptr<Keyboard> keyboard;
			DevicePtr device;
		};

		struct GamepadDevice {
			std::unique_ptr<GamePad> gamePad;
			DevicePtr device;
			SDL_GUID guid;
		};

		KeyboardDevice createKeyboardDevice(const std::string& name, KeyboardBindings defaults);
		Keyboard* findKeyboardBackend(Device& device) const;
		GamePad* findGamepadBackend(Device& device) const;
		BindingSet getConfiguredBindings(const std::string& guid, BindingSet defaults) const;
		void controllerDeviceAddedEvent(const SDL_GamepadDeviceEvent& deviceEvent);
		void controllerDeviceRemovedEvent(const SDL_GamepadDeviceEvent& deviceEvent);
		void clearInput(KeyboardDevice& keyboardDevice);
		void clearInput(GamepadDevice& gamepadDevice);

		std::vector<DeviceBinding> bindings_;
		PersistenceCallback persist_;
		Device* mappingDevice_ = nullptr;
		KeyboardDevice keyboard1_;
		KeyboardDevice keyboard2_;
		std::vector<GamepadDevice> devices_;
	};

}

#endif
