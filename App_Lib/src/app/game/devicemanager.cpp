#include "devicemanager.h"

#include <sdl/gamecontroller.h>

#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <string>
#include <utility>

namespace app::game {

	DeviceManager::DeviceManager(std::vector<DeviceBinding> bindings, PersistenceCallback persist)
		: bindings_{std::move(bindings)}
		, persist_{std::move(persist)} {

		keyboard1_ = createKeyboardDevice("Keyboard 1", getDefaultKeyboard1Bindings());
		keyboard2_ = createKeyboardDevice("Keyboard 2", getDefaultKeyboard2Bindings());
	}

	DeviceManager::KeyboardDevice DeviceManager::createKeyboardDevice(const std::string& name, KeyboardBindings defaults) {
		auto configured = getConfiguredBindings(name, defaults);
		auto* bindings = std::get_if<KeyboardBindings>(&configured);

		KeyboardDevice keyboardDevice{
			.keyboard = std::make_unique<Keyboard>(
				name,
				bindings ? *bindings : defaults,
				defaults),
			.device = std::make_shared<Device>(),
		};

		keyboardDevice.device->guid_ = name;
		keyboardDevice.device->name_ = name;
		keyboardDevice.device->connected_ = true;
		keyboardDevice.device->input_ = {};
		return keyboardDevice;
	}

	void DeviceManager::beginMapping(Device& device) {
		if (!device.isConnected()) {
			return;
		}
		if (mappingDevice_ && mappingDevice_ != &device) {
			cancelMapping(*mappingDevice_);
		}
		mappingDevice_ = &device;
		if (auto* keyboard = findKeyboardBackend(device)) {
			keyboard->beginMapping();
			device.input_ = keyboard->getInput();
		} else if (auto* gamePad = findGamepadBackend(device)) {
			gamePad->beginMapping();
			device.input_ = gamePad->getInput();
		} else {
			mappingDevice_ = nullptr;
			return;
		}
		device.invokeInputReset();
	}

	void DeviceManager::resetMapping(Device& device) {
		if (mappingDevice_ != &device) {
			return;
		}
		if (auto* keyboard = findKeyboardBackend(device)) {
			keyboard->resetMapping();
			device.input_ = keyboard->getInput();
		} else if (auto* gamePad = findGamepadBackend(device)) {
			gamePad->resetMapping();
			device.input_ = gamePad->getInput();
		}
	}

	void DeviceManager::cancelMapping(Device& device) {
		if (mappingDevice_ != &device) {
			return;
		}
		if (auto* keyboard = findKeyboardBackend(device)) {
			keyboard->cancelMapping();
			device.input_ = keyboard->getInput();
		} else if (auto* gamePad = findGamepadBackend(device)) {
			gamePad->cancelMapping();
			device.input_ = gamePad->getInput();
		}
		mappingDevice_ = nullptr;
	}

	std::optional<MappingStatus> DeviceManager::getMappingStatus(Device& device) const {
		if (auto* keyboard = findKeyboardBackend(device)) {
			return keyboard->getMappingStatus();
		}
		if (auto* gamePad = findGamepadBackend(device)) {
			return gamePad->getMappingStatus();
		}
		return std::nullopt;
	}

	std::array<std::string, 5> DeviceManager::getMappingLabels(Device& device) const {
		if (auto* keyboard = findKeyboardBackend(device)) {
			return keyboard->getMappingLabels();
		}
		if (auto* gamePad = findGamepadBackend(device)) {
			return gamePad->getMappingLabels();
		}
		return {};
	}

	Keyboard* DeviceManager::findKeyboardBackend(Device& device) const {
		if (keyboard1_.device.get() == &device) {
			return keyboard1_.keyboard.get();
		}
		if (keyboard2_.device.get() == &device) {
			return keyboard2_.keyboard.get();
		}
		return nullptr;
	}

	GamePad* DeviceManager::findGamepadBackend(Device& device) const {
		auto found = std::ranges::find_if(devices_, [&device](const GamepadDevice& pair) {
			return pair.device.get() == &device;
		});
		return found == devices_.end() ? nullptr : found->gamePad.get();
	}

	BindingSet DeviceManager::getConfiguredBindings(const std::string& guid, BindingSet defaults) const {
		auto found = std::ranges::find(bindings_, guid, &DeviceBinding::deviceGuid);
		return found == bindings_.end() ? std::move(defaults) : found->bindings;
	}

	bool DeviceManager::eventUpdate(const SDL_Event& windowEvent) {
		switch (windowEvent.type) {
			case SDL_EVENT_GAMEPAD_ADDED:
				controllerDeviceAddedEvent(windowEvent.gdevice);
				break;
			case SDL_EVENT_GAMEPAD_REMOVED:
				controllerDeviceRemovedEvent(windowEvent.gdevice);
				break;
		}

		if (auto* mappingDevice = mappingDevice_) {
			if (windowEvent.type == SDL_EVENT_KEY_DOWN && windowEvent.key.repeat == 0
				&& windowEvent.key.key == SDLK_ESCAPE) {
				cancelMapping(*mappingDevice);
				return true;
			}
			if (auto* keyboard = findKeyboardBackend(*mappingDevice); keyboard && keyboard->captureMappingEvent(windowEvent)) {
				mappingDevice->input_ = keyboard->getInput();
				return true;
			}
			if (auto* gamePad = findGamepadBackend(*mappingDevice); gamePad && gamePad->captureMappingEvent(windowEvent)) {
				mappingDevice->input_ = gamePad->getInput();
				return true;
			}
		}

		std::optional<InputCommand> command;
		if (keyboard1_.keyboard->eventUpdate(windowEvent, command)) {
			keyboard1_.device->input_ = keyboard1_.keyboard->getInput();
			if (command) {
				keyboard1_.device->invokeCommand(*command);
			}
		}
		if (keyboard2_.keyboard->eventUpdate(windowEvent, command)) {
			keyboard2_.device->input_ = keyboard2_.keyboard->getInput();
			if (command) {
				keyboard2_.device->invokeCommand(*command);
			}
		}
		for (auto& [gamePad, device, guid] : devices_) {
			if (gamePad->eventUpdate(windowEvent, command)) {
				device->input_ = gamePad->getInput();
				if (command) {
					device->invokeCommand(*command);
				}
			}
		}
		return false;
	}

	void DeviceManager::tick() {
		if (auto* mappingDevice = mappingDevice_) {
			if (auto* keyboard = findKeyboardBackend(*mappingDevice)) {
				switch (keyboard->pollMappingInput()) {
					case MappingPollResult::Captured:
						mappingDevice->input_ = keyboard->getInput();
						break;
					case MappingPollResult::CancelRequested:
						cancelMapping(*mappingDevice);
						break;
					case MappingPollResult::None:
						break;
				}
			}
		}
	}

	void DeviceManager::clearInput(KeyboardDevice& keyboardDevice) {
		keyboardDevice.keyboard->resetInput();
		keyboardDevice.device->input_ = keyboardDevice.keyboard->getInput();
		keyboardDevice.device->invokeInputReset();
	}

	void DeviceManager::clearInput(GamepadDevice& gamepadDevice) {
		gamepadDevice.gamePad->resetInput();
		gamepadDevice.device->input_ = gamepadDevice.gamePad->getInput();
		gamepadDevice.device->invokeInputReset();
	}

	void DeviceManager::clearInput() {
		clearInput(keyboard1_);
		clearInput(keyboard2_);
		for (auto& gamepadDevice : devices_) {
			clearInput(gamepadDevice);
		}
	}

	std::vector<DevicePtr> DeviceManager::getAllDevicesAvailable() const {
		std::vector<DevicePtr> devices;
		devices.reserve(2 + devices_.size());
		devices.push_back(keyboard1_.device);
		devices.push_back(keyboard2_.device);
		for (const auto& pair : devices_) {
			if (pair.device->isConnected()) {
				devices.push_back(pair.device);
			}
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
		auto found = std::ranges::find_if(devices_, [&guid](const GamepadDevice& pair) {
			return pair.device->getGuid() == guid;
		});
		return found == devices_.end() ? nullptr : found->device;
	}

	DeviceBindingsPersistenceResult DeviceManager::saveMapping(Device& device) {
		if (mappingDevice_ != &device) {
			return std::unexpected{"This device is not being configured."};
		}

		std::optional<BindingSet> draft;
		if (auto* keyboard = findKeyboardBackend(device)) {
			if (auto value = keyboard->getDraftBindings()) {
				draft = *value;
			}
		} else if (auto* gamePad = findGamepadBackend(device)) {
			if (auto value = gamePad->getDraftBindings()) {
				draft = *value;
			}
		}
		if (!draft) {
			return std::unexpected{"No binding draft is available."};
		}
		auto status = getMappingStatus(device);
		if (!status || status->completed < status->total) {
			return std::unexpected{"Capture all five actions before saving."};
		}
		auto validation = std::visit([](const auto& value) { return validateBindings(value); }, *draft);
		if (!validation) {
			return validation;
		}

		auto candidate = bindings_;
		auto found = std::ranges::find(candidate, device.getGuid(), &DeviceBinding::deviceGuid);
		if (found == candidate.end()) {
			candidate.push_back({device.getGuid(), *draft});
		} else {
			found->bindings = *draft;
		}
		if (persist_) {
			if (auto result = persist_(candidate); !result) {
				return result;
			}
		}

		BindingsValidationResult commit;
		if (auto* keyboard = findKeyboardBackend(device)) {
			commit = keyboard->commitMapping();
			device.input_ = keyboard->getInput();
		} else {
			auto* gamePad = findGamepadBackend(device);
			commit = gamePad ? gamePad->commitMapping() : BindingsValidationResult{std::unexpected{"Device backend is unavailable."}};
			if (gamePad) {
				device.input_ = gamePad->getInput();
			}
		}
		if (!commit) {
			return std::unexpected{commit.error()};
		}
		bindings_ = std::move(candidate);
		mappingDevice_ = nullptr;
		return {};
	}

	void DeviceManager::controllerDeviceAddedEvent(const SDL_GamepadDeviceEvent& deviceEvent) {
		auto gameController = sdl::GameController::addController(deviceEvent.which);
		auto found = std::ranges::find_if(devices_, [&](const GamepadDevice& pair) {
			return !pair.device->isConnected() && pair.guid == gameController.getGuid();
		});
		if (found != devices_.end()) {
			found->gamePad->setGameController(std::move(gameController));
			found->device->input_ = {};
			found->device->invokeInputReset();
			spdlog::info("GamePad reconnected: {} {} {}", found->gamePad->getInstanceId(), found->gamePad->getGuid(), found->gamePad->getName());
			found->device->invokeConnection(true);
			return;
		}

		auto guid = gameController.getGuid();
		auto guidString = sdl::guidToString(guid);
		auto configured = getConfiguredBindings(guidString, getDefaultGamepadBindings());
		auto bindings = std::holds_alternative<GamepadBindings>(configured)
			? std::get<GamepadBindings>(configured)
			: getDefaultGamepadBindings();
		auto& pair = devices_.emplace_back(GamepadDevice{
			.gamePad = std::make_unique<GamePad>(std::move(gameController), bindings),
			.device = std::make_shared<Device>(),
			.guid = guid,
		});
		pair.device->connected_ = true;
		pair.device->guid_ = pair.gamePad->getGuid();
		pair.device->input_ = {};
		pair.device->name_ = pair.gamePad->getName();
		spdlog::info("GamePad added: {} {} {}", pair.gamePad->getInstanceId(), pair.gamePad->getGuid(), pair.gamePad->getName());
		deviceConnected(pair.device);
	}

	void DeviceManager::controllerDeviceRemovedEvent(const SDL_GamepadDeviceEvent& deviceEvent) {
		auto found = std::ranges::find_if(devices_, [instanceId = deviceEvent.which](const GamepadDevice& pair) {
			return instanceId == pair.gamePad->getInstanceId();
		});
		if (found != devices_.end()) {
			cancelMapping(*found->device);
			clearInput(*found);
			found->device->invokeConnection(false);
			spdlog::info("GamePad removed: {} {} {}", found->gamePad->getInstanceId(), found->gamePad->getGuid(), found->gamePad->getName());
		}
	}

}
