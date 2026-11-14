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

		keyboard1_ = createKeyboardDevice("Keyboard 1", getDefaultKeyboard1Bindings(), DeviceColors[0]);
		keyboard2_ = createKeyboardDevice("Keyboard 2", getDefaultKeyboard2Bindings(), DeviceColors[1]);
	}

	DeviceManager::KeyboardDevice DeviceManager::createKeyboardDevice(
		const std::string& name, KeyboardBindings defaults, sdl::Color color) {

		auto configured = getConfiguredBindings(name, defaults);
		auto* bindings = std::get_if<KeyboardBindings>(&configured);

		KeyboardDevice keyboardDevice{
			.keyboard = std::make_unique<Keyboard>(
				name,
				bindings ? *bindings : defaults,
				defaults),
			.device = std::make_shared<Device>(name, name, DeviceKind::Keyboard, color),
		};

		keyboardDevice.device->connected_ = true;
		return keyboardDevice;
	}

	DeviceManager::GamepadDevice DeviceManager::createGamepadDevice(sdl::GameController&& gameController, const DeviceBinding& configuration) {
		const auto guid = gameController.getGuid();
		const auto defaults = getDefaultGamepadBindings();
		const auto configured = getConfiguredBindings(configuration.deviceGuid, defaults);
		const auto* bindings = std::get_if<GamepadBindings>(&configured);

		auto gamePad = std::make_unique<GamePad>(
			std::move(gameController),
			bindings ? *bindings : defaults
		);
		auto device = std::make_shared<Device>(
			gamePad->getName(),
			configuration.deviceGuid,
			DeviceKind::Gamepad,
			configuration.color
		);
		GamepadDevice gamepadDevice{
			.gamePad = std::move(gamePad),
			.device = std::move(device),
			.guid = guid,
		};
		gamepadDevice.device->connected_ = true;
		return gamepadDevice;
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
		} else if (auto* gamePad = findGamepadBackend(device)) {
			gamePad->beginMapping();
		} else {
			mappingDevice_ = nullptr;
			return;
		}
		device.input_ = {};
		device.invokeInputReset();
	}

	void DeviceManager::resetMapping(Device& device) {
		if (mappingDevice_ != &device) {
			return;
		}
		if (auto* keyboard = findKeyboardBackend(device)) {
			keyboard->resetMapping();
		} else if (auto* gamePad = findGamepadBackend(device)) {
			gamePad->resetMapping();
		}
		device.input_ = {};
	}

	void DeviceManager::cancelMapping(Device& device) {
		if (mappingDevice_ != &device) {
			return;
		}
		if (auto* keyboard = findKeyboardBackend(device)) {
			keyboard->cancelMapping();
		} else if (auto* gamePad = findGamepadBackend(device)) {
			gamePad->cancelMapping();
		}
		device.input_ = {};
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
		auto* configuration = findConfiguration(guid);
		return configuration ? configuration->bindings : std::move(defaults);
	}

	DeviceBinding* DeviceManager::findConfiguration(const std::string& deviceGuid) {
		auto found = std::ranges::find(bindings_, deviceGuid, &DeviceBinding::deviceGuid);
		return found == bindings_.end() ? nullptr : &*found;
	}

	const DeviceBinding* DeviceManager::findConfiguration(const std::string& deviceGuid) const {
		auto found = std::ranges::find(bindings_, deviceGuid, &DeviceBinding::deviceGuid);
		return found == bindings_.end() ? nullptr : &*found;
	}

	sdl::Color DeviceManager::nextAvailableColor() const {
		for (auto color : DeviceColors) {
			const bool configured = std::ranges::any_of(bindings_, [color](const DeviceBinding& binding) {
				return binding.color == color;
			});
			const bool active = keyboard1_.device && keyboard1_.device->getColor() == color
				|| keyboard2_.device && keyboard2_.device->getColor() == color
				|| std::ranges::any_of(devices_, [color](const GamepadDevice& device) {
					return device.device->getColor() == color;
				});
			if (!configured && !active) {
				return color;
			}
		}
		return DeviceColors[(bindings_.size() + devices_.size()) % DeviceColors.size()];
	}

	std::string DeviceManager::nextLogicalGuid(std::string_view hardwareGuid) const {
		auto isUsed = [this](std::string_view candidate) {
			return std::ranges::any_of(bindings_, [candidate](const DeviceBinding& binding) {
				return binding.deviceGuid == candidate;
			}) || std::ranges::any_of(devices_, [candidate](const GamepadDevice& device) {
				return device.device->getGuid() == candidate;
			});
		};
		if (!isUsed(hardwareGuid)) {
			return std::string{hardwareGuid};
		}
		for (int slot = 2;; ++slot) {
			auto candidate = std::string{hardwareGuid} + "#" + std::to_string(slot);
			if (!isUsed(candidate)) {
				return candidate;
			}
		}
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
				return true;
			}
			if (auto* gamePad = findGamepadBackend(*mappingDevice); gamePad && gamePad->captureMappingEvent(windowEvent)) {
				return true;
			}
		}

		if (auto event = keyboard1_.keyboard->eventUpdate(windowEvent)) {
			keyboard1_.device->apply(*event);
		}
		if (auto event = keyboard2_.keyboard->eventUpdate(windowEvent)) {
			keyboard2_.device->apply(*event);
		}
		for (auto& [gamePad, device, guid] : devices_) {
			if (auto event = gamePad->eventUpdate(windowEvent)) {
				device->apply(*event);
			}
		}
		return false;
	}

	void DeviceManager::tick() {
		if (auto* mappingDevice = mappingDevice_) {
			if (auto* keyboard = findKeyboardBackend(*mappingDevice)) {
				switch (keyboard->pollMappingInput()) {
				case MappingPollResult::Captured:
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
		keyboardDevice.device->input_ = {};
		keyboardDevice.device->invokeInputReset();
	}

	void DeviceManager::clearInput(GamepadDevice& gamepadDevice) {
		gamepadDevice.device->input_ = {};
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
			std::string hardwareGuid = device.getGuid();
			if (auto gamePad = findGamepadBackend(device)) {
				hardwareGuid = gamePad->getGuid();
			}
			candidate.push_back({device.getGuid(), *draft, std::move(hardwareGuid), device.getKind(), device.getColor()});
		} else {
			found->bindings = *draft;
			found->hardwareGuid = device.getKind() == DeviceKind::Keyboard
				? device.getGuid()
				: findGamepadBackend(device)->getGuid();
			found->kind = device.getKind();
			found->color = device.getColor();
		}
		if (persist_) {
			if (auto result = persist_(candidate); !result) {
				return result;
			}
		}

		BindingsValidationResult commit;
		if (auto* keyboard = findKeyboardBackend(device)) {
			commit = keyboard->commitMapping();
			device.input_ = {};
		} else {
			auto* gamePad = findGamepadBackend(device);
			commit = gamePad ? gamePad->commitMapping() : BindingsValidationResult{std::unexpected{"Device backend is unavailable."}};
			if (gamePad) {
				device.input_ = {};
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
		const auto hardwareGuid = sdl::guidToString(gameController.getGuid());
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

		auto configured = std::ranges::find_if(bindings_, [&](const DeviceBinding& binding) {
			const auto physicalGuid = binding.hardwareGuid.empty() ? binding.deviceGuid : binding.hardwareGuid;
			const bool instantiated = std::ranges::any_of(devices_, [&](const GamepadDevice& device) {
				return device.device->getGuid() == binding.deviceGuid;
			});
			return binding.kind == DeviceKind::Gamepad && physicalGuid == hardwareGuid && !instantiated;
		});
		if (configured == bindings_.end()) {
			DeviceBinding binding{
				nextLogicalGuid(hardwareGuid),
				getDefaultGamepadBindings(),
				hardwareGuid,
				DeviceKind::Gamepad,
				nextAvailableColor(),
			};
			auto candidate = bindings_;
			candidate.push_back(binding);
			if (!persist_ || persist_(candidate)) {
				bindings_ = std::move(candidate);
				configured = std::prev(bindings_.end());
			} else {
				spdlog::error("Could not persist gamepad color for {}", binding.deviceGuid);
				bindings_.push_back(std::move(binding));
				configured = std::prev(bindings_.end());
			}
		} else {
			if (configured->color == sdl::Color{}) {
				configured->color = nextAvailableColor();
			}
			configured->hardwareGuid = hardwareGuid;
			configured->kind = DeviceKind::Gamepad;
		}

		auto& pair = devices_.emplace_back(createGamepadDevice(std::move(gameController), *configured));
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
