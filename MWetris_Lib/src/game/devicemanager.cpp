#include "devicemanager.h"

#include <SDL.h>

#include <string>
#include <spdlog/spdlog.h>

#include <algorithm>

namespace mwetris::game {

	DeviceManager::DeviceManager() {
		keyboard1_ = std::make_shared<game::Keyboard>("Keyboard 1", SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_UP, SDLK_RCTRL);
		keyboard2_ = std::make_shared<game::Keyboard>("Keyboard 2", SDLK_s, SDLK_a, SDLK_d, SDLK_w, SDLK_LCTRL);
	}

	void DeviceManager::eventUpdate(const SDL_Event& windowEvent) {
		switch (windowEvent.type) {
			case SDL_CONTROLLERDEVICEADDED:
				controllerDeviceAddedEvent(windowEvent.cdevice);
				break;
			case SDL_CONTROLLERDEVICEREMOVED:
				controllerDeviceRemovedEvent(windowEvent.cdevice);
				break;
		}
		if (searchForDevice_) {
			switch (windowEvent.type) {
				case SDL_KEYDOWN:
					if (keyboard1_->eventUpdate(windowEvent)) {
						stopSearchForDevice();
						deviceFound(keyboard1_);
					} else if (keyboard2_->eventUpdate(windowEvent)) {
						stopSearchForDevice();
						deviceFound(keyboard2_);
					}
				case SDL_CONTROLLERBUTTONDOWN:
					for (auto& gamePad : gamePads_) {
						if (gamePad->eventUpdate(windowEvent)) {
							stopSearchForDevice();
							deviceFound(gamePad);
						}
					}
					break;
			}
		} else {
			keyboard1_->eventUpdate(windowEvent);
			keyboard2_->eventUpdate(windowEvent);
			for (auto& gamePad : gamePads_) {
				gamePad->eventUpdate(windowEvent);
			}
		}
	}

	void DeviceManager::searchForDevice() {
		searchForDevice_ = true;
	}

	void DeviceManager::stopSearchForDevice() {
		searchForDevice_ = false;
	}

	std::vector<DevicePtr> DeviceManager::getAllDevicesAvailable() const {
		std::vector<DevicePtr> devices;
		devices.reserve(2 + gamePads_.size());
		devices.push_back(keyboard1_);
		devices.push_back(keyboard2_);
		devices.insert(devices.end(), gamePads_.begin(), gamePads_.end());
		return devices;
	}

	DevicePtr DeviceManager::getDefaultDevice1() const {
		return keyboard1_;
	}

	DevicePtr DeviceManager::getDefaultDevice2() const {
		return keyboard2_;
	}

	void DeviceManager::controllerDeviceAddedEvent(const SDL_ControllerDeviceEvent& deviceEvent) {
		auto gameController = sdl::GameController::addController(deviceEvent.which);
		const auto& gamePad = gamePads_.emplace_back(std::make_shared<GamePad>(std::move(gameController)));
		spdlog::info("GamePad added: {} {} {}", gamePad->getInstanceId(), gamePad->getGuid(), gamePad->getName());
	}

	void DeviceManager::controllerDeviceRemovedEvent(const SDL_ControllerDeviceEvent& deviceEvent) {
		std::erase_if(gamePads_, [instanceId = deviceEvent.which](const std::shared_ptr<GamePad>& gamePad) {
			if (gamePad->getInstanceId() == instanceId) {
				spdlog::info("GamePad removed: {} {} {}", gamePad->getInstanceId(), gamePad->getGuid(), gamePad->getName());
				return true;
			}
			return false;
		});
	}

}
