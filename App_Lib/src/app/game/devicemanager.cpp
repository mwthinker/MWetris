#include "devicemanager.h"

#include <sdl/gamecontroller.h>

#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

#include <string>
#include <algorithm>

namespace app::game {

	DeviceManager::DeviceManager() {
		keyboard1_ = createKeyboardDevice(std::make_unique<game::Keyboard>("Keyboard 1",
			KeyboardMapping{
				.down = SDLK_DOWN,
				.right = SDLK_RIGHT,
				.left = SDLK_LEFT,
				.rotate = SDLK_UP,
				.downGround = SDLK_RCTRL
			}
		));
		keyboard2_ = createKeyboardDevice(std::make_unique<game::Keyboard>("Keyboard 2",
			KeyboardMapping{
				.down = SDLK_S,
				.right = SDLK_D,
				.left = SDLK_A,
				.rotate = SDLK_W,
				.downGround = SDLK_LCTRL
			}
		));
	}

	DeviceManager::KeyboardDevice DeviceManager::createKeyboardDevice(std::unique_ptr<Keyboard> keyboard) {
		KeyboardDevice keyboardDevice;
		keyboardDevice.keyboard = std::move(keyboard);
		keyboardDevice.device = std::make_shared<Device>();
		keyboardDevice.device->guid_ = keyboardDevice.keyboard->getName();
		keyboardDevice.device->name_ = keyboardDevice.keyboard->getName();
		keyboardDevice.device->connected_ = true;
		keyboardDevice.device->input_ = {};
		return keyboardDevice;
	}

	void DeviceManager::eventUpdate(const SDL_Event& windowEvent) {
		switch (windowEvent.type) {
			case SDL_EVENT_GAMEPAD_ADDED:
				controllerDeviceAddedEvent(windowEvent.gdevice);
				break;
			case SDL_EVENT_GAMEPAD_REMOVED:
				controllerDeviceRemovedEvent(windowEvent.gdevice);
				break;
		}
		if (keyboard1_.keyboard->eventUpdate(windowEvent)) {
			keyboard1_.device->input_ = keyboard1_.keyboard->getInput();
		}
		if (keyboard2_.keyboard->eventUpdate(windowEvent)) {
			keyboard2_.device->input_ = keyboard2_.keyboard->getInput();
		}
		for (auto& [gamePad, device, _] : devices_) {
			if (gamePad->eventUpdate(windowEvent)) {
				device->input_ = gamePad->getInput();
			}
		}
	}

	void DeviceManager::tick() {
		keyboard1_.keyboard->tick();
		keyboard2_.keyboard->tick();
		for (auto& [gamePad, device, _] : devices_) {
			// TODO! Implement GamePad tick logic
		}
	}

	std::vector<DevicePtr> DeviceManager::getAllDevicesAvailable() const {
		std::vector<DevicePtr> devices;
		devices.reserve(2 + devices_.size());
		devices.push_back(keyboard1_.device);
		devices.push_back(keyboard2_.device);
		for (auto& pair : devices_) {
			devices.push_back(pair.device);
		}
		return devices;
	}

	DevicePtr DeviceManager::getDefaultDevice1() const {
		return keyboard1_.device;
	}

	DevicePtr DeviceManager::getDefaultDevice2() const {
		return keyboard2_.device;
	}

	DevicePtr DeviceManager::findDevice(const std::string& guid) const {
		if (keyboard1_.device->getGuid() == guid) {
			return keyboard1_.device;
		}
		if (keyboard2_.device->getGuid() == guid) {
			return keyboard2_.device;
		}

		auto sdlGuid = SDL_StringToGUID(guid.c_str());

		auto it = std::find_if(devices_.begin(), devices_.end(), [sdlGuid](const Pair& pair) {
			return pair.guid == sdlGuid;
		});
		if (it != devices_.end()) {
			return it->device;
		}
		return keyboard1_.device;
	}

	void DeviceManager::controllerDeviceAddedEvent(const SDL_GamepadDeviceEvent& deviceEvent) {
		auto gameController = sdl::GameController::addController(deviceEvent.which);
		auto it = std::find_if(devices_.begin(), devices_.end(), [&](const Pair& pair) {
			return !pair.device->isConnected() && pair.guid == gameController.getGuid();
		});
		if (it != devices_.end()) {
			it->gamePad->setGameController(std::move(gameController));
			spdlog::info("GamePad reconnected: {} {} {}", it->gamePad->getInstanceId(), it->gamePad->getGuid(), it->gamePad->getName());
			it->device->invokeConnection(true);
		} else {
			auto guid = gameController.getGuid();

			auto& pair = devices_.emplace_back(Pair{
				.gamePad = std::make_unique<GamePad>(std::move(gameController)),
				.device = std::make_shared<Device>(),
				.guid = guid
			});
			pair.device->connected_ = true;
			pair.device->guid_ = pair.gamePad->getGuid();
			pair.device->input_ = {};
			pair.device->name_ = pair.gamePad->getName();

			spdlog::info("GamePad added: {} {} {}", pair.gamePad->getInstanceId(), pair.gamePad->getGuid(), pair.gamePad->getName());
			deviceConnected(pair.device);
		}
	}

	void DeviceManager::controllerDeviceRemovedEvent(const SDL_GamepadDeviceEvent& deviceEvent) {
		auto it = std::find_if(devices_.begin(), devices_.end(), [instanceId = deviceEvent.which](const Pair& pair) {
			return instanceId == pair.gamePad->getInstanceId();
		});
		if (it != devices_.end()) {
			it->device->invokeConnection(false);
			spdlog::info("GamePad removed: {} {} {}", it->gamePad->getInstanceId(), it->gamePad->getGuid(), it->gamePad->getName());
		}
	}

}
