#include <gtest/gtest.h>

#include <app/game/devicemanager.h>

#include <ostream>

namespace app::game {

	namespace {

		SDL_Event keyboardEvent(SDL_Keycode key, SDL_EventType type = SDL_EVENT_KEY_DOWN) {
			SDL_Event event{};
			event.key = {
				.type = type,
				.key = key,
			};
			return event;
		}

		SDL_Event gamepadDeviceEvent(SDL_JoystickID instanceId, SDL_EventType type) {
			SDL_Event event = {};
			event.gdevice = {
				.type = type,
				.which = instanceId,
			};
			return event;
		}

		SDL_Event gamepadButtonEvent(SDL_JoystickID instanceId, SDL_GamepadButton button,
			SDL_EventType type = SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
			SDL_Event event{};
			event.gbutton = {
				.type = type,
				.which = instanceId,
				.button = static_cast<Uint8>(button),
			};
			return event;
		}

		void captureCompleteKeyboardMapping(DeviceManager& manager, const std::array<SDL_Keycode, 5>& keys) {
			for (auto key : keys) {
				EXPECT_TRUE(manager.eventUpdate(keyboardEvent(key)));
			}
		}

	}

	enum class InputDeviceType {
		Keyboard,
		Gamepad,
	};

	// Lets gtest print readable input-device names in diagnostics.
	void PrintTo(InputDeviceType type, std::ostream* stream) {
		*stream << (type == InputDeviceType::Keyboard ? "Keyboard" : "Gamepad");
	}

	class DeviceInputContractTest : public ::testing::TestWithParam<InputDeviceType> {
	protected:
		void SetUp() override {
			if (GetParam() == InputDeviceType::Keyboard) {
				manager_ = std::make_unique<DeviceManager>();
				device_ = manager_->getDefaultDevice1();
				return;
			}

			ASSERT_TRUE(SDL_InitSubSystem(SDL_INIT_GAMEPAD));
			SDL_INIT_INTERFACE(&description_);
			description_.type = SDL_JOYSTICK_TYPE_GAMEPAD;
			description_.vendor_id = 0x4321;
			description_.product_id = 0x8765;
			description_.nbuttons = SDL_GAMEPAD_BUTTON_COUNT;
			description_.name = "MWetris input contract gamepad";

			gamepadInstanceId_ = SDL_AttachVirtualJoystick(&description_);
			ASSERT_NE(gamepadInstanceId_, 0u) << SDL_GetError();
			manager_ = std::make_unique<DeviceManager>();
			manager_->eventUpdate(gamepadDeviceEvent(gamepadInstanceId_, SDL_EVENT_GAMEPAD_ADDED));

			auto guid = sdl::guidToString(SDL_GetJoystickGUIDForID(gamepadInstanceId_));
			device_ = manager_->findDevice(guid);
			ASSERT_TRUE(device_);
		}

		void TearDown() override {
			if (gamepadInstanceId_ != 0) {
				SDL_DetachVirtualJoystick(gamepadInstanceId_);
			}
			manager_.reset();
			if (GetParam() == InputDeviceType::Gamepad) {
				SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
			}
		}

		void sendLeftDown() {
			if (GetParam() == InputDeviceType::Keyboard) {
				manager_->eventUpdate(keyboardEvent(SDLK_LEFT));
			} else {
				manager_->eventUpdate(gamepadButtonEvent(gamepadInstanceId_, SDL_GAMEPAD_BUTTON_DPAD_LEFT));
			}
		}

		void sendLeftUp() {
			if (GetParam() == InputDeviceType::Keyboard) {
				manager_->eventUpdate(keyboardEvent(SDLK_LEFT, SDL_EVENT_KEY_UP));
			} else {
				manager_->eventUpdate(gamepadButtonEvent(
					gamepadInstanceId_, SDL_GAMEPAD_BUTTON_DPAD_LEFT, SDL_EVENT_GAMEPAD_BUTTON_UP));
			}
		}

		SDL_VirtualJoystickDesc description_{};
		SDL_JoystickID gamepadInstanceId_ = 0;
		std::unique_ptr<DeviceManager> manager_;
		DevicePtr device_;
	};

	TEST_P(DeviceInputContractTest, pressCreatesOneFrameTransition) {
		// Given
		ASSERT_TRUE(device_);

		// When
		sendLeftDown();

		// Then
		const auto left = device_->getInput().left;
		EXPECT_TRUE(left.pressed);
		EXPECT_TRUE(left.held);
		EXPECT_FALSE(left.released);
	}

	TEST_P(DeviceInputContractTest, tickKeepsHeldAndClearsPressed) {
		// Given
		sendLeftDown();

		// When
		manager_->tick();

		// Then
		const auto left = device_->getInput().left;
		EXPECT_FALSE(left.pressed);
		EXPECT_TRUE(left.held);
		EXPECT_FALSE(left.released);
	}

	TEST_P(DeviceInputContractTest, repeatedDownDoesNotRetriggerPressed) {
		// Given
		sendLeftDown();
		manager_->tick();

		// When
		sendLeftDown();

		// Then
		const auto left = device_->getInput().left;
		EXPECT_FALSE(left.pressed);
		EXPECT_TRUE(left.held);
		EXPECT_FALSE(left.released);
	}

	TEST_P(DeviceInputContractTest, releaseCreatesOneFrameTransition) {
		// Given
		sendLeftDown();
		manager_->tick();

		// When
		sendLeftUp();

		// Then
		auto left = device_->getInput().left;
		EXPECT_FALSE(left.pressed);
		EXPECT_FALSE(left.held);
		EXPECT_TRUE(left.released);
	}

	TEST_P(DeviceInputContractTest, tickClearsReleasedTransition) {
		// Given
		sendLeftDown();
		manager_->tick();
		sendLeftUp();

		// When
		manager_->tick();

		// Then
		const auto left = device_->getInput().left;
		EXPECT_FALSE(left.pressed);
		EXPECT_FALSE(left.held);
		EXPECT_FALSE(left.released);
	}

	INSTANTIATE_TEST_SUITE_P(
		DeviceTypes,
		DeviceInputContractTest,
		::testing::Values(InputDeviceType::Keyboard, InputDeviceType::Gamepad),
		[](const ::testing::TestParamInfo<InputDeviceType>& info) {
			return info.param == InputDeviceType::Keyboard ? "Keyboard" : "Gamepad";
		});

	TEST(DeviceManagerTest, createsDistinctDefaultDevices) {
		// Given
		DeviceManager manager;

		// When
		auto device1 = manager.getDefaultDevice1();
		auto device2 = manager.getDefaultDevice2();

		// Then
		ASSERT_TRUE(device1);
		ASSERT_TRUE(device1->isConnected());
		ASSERT_TRUE(device2);
		ASSERT_TRUE(device2->isConnected());
		EXPECT_NE(device1, device2);
		EXPECT_NE(device1->getGuid(), device2->getGuid());
		EXPECT_NE(device1->getName(), device2->getName());
	}

	TEST(DeviceManagerTest, selectedDeviceExclusivelyCapturesWithoutGameplayInput) {
		// Given
		DeviceManager manager;
		auto device1 = manager.getDefaultDevice1();
		auto device2 = manager.getDefaultDevice2();
		manager.beginMapping(*device1);

		// When
		EXPECT_TRUE(manager.eventUpdate(keyboardEvent(SDLK_A)));

		// Then
		ASSERT_TRUE(manager.getMappingStatus(*device1));
		EXPECT_EQ(manager.getMappingStatus(*device1)->completed, 1);
		EXPECT_EQ(device1->getInput(), Input{});
		EXPECT_EQ(device2->getInput(), Input{});
	}

	TEST(DeviceManagerTest, beginMappingStartsKeyboardDraftOver) {
		// Given
		DeviceManager manager;
		auto device = manager.getDefaultDevice1();
		manager.beginMapping(*device);
		manager.eventUpdate(keyboardEvent(SDLK_Q));
		manager.eventUpdate(keyboardEvent(SDLK_E));
		ASSERT_EQ(manager.getMappingStatus(*device)->completed, 2);

		// When
		manager.beginMapping(*device);

		// Then
		ASSERT_TRUE(manager.getMappingStatus(*device));
		EXPECT_EQ(manager.getMappingStatus(*device)->completed, 0);
	}

	TEST(DeviceManagerTest, savesCompleteDraftAfterPersistence) {
		// Given
		std::vector<DeviceBinding> persisted;
		int persistenceCalls = 0;
		DeviceManager manager{{}, [&](const std::vector<DeviceBinding>& candidate) -> DeviceBindingsPersistenceResult {
			persisted = candidate;
			++persistenceCalls;
			return {};
		}};
		auto device = manager.getDefaultDevice1();
		manager.beginMapping(*device);
		captureCompleteKeyboardMapping(manager, {SDLK_J, SDLK_L, SDLK_K, SDLK_I, SDLK_SPACE});
		ASSERT_EQ(manager.getMappingStatus(*device)->completed, 5);

		// When
		auto result = manager.saveMapping(*device);

		// Then
		ASSERT_TRUE(result);
		EXPECT_EQ(persistenceCalls, 1);
		ASSERT_EQ(persisted.size(), 1);
		EXPECT_EQ(persisted.front().deviceGuid, "Keyboard 1");
		EXPECT_FALSE(manager.getMappingStatus(*device));
	}

	TEST(DeviceManagerTest, savedMappingUpdatesGameplayInput) {
		// Given
		DeviceManager manager;
		auto device = manager.getDefaultDevice1();
		manager.beginMapping(*device);
		captureCompleteKeyboardMapping(manager, {SDLK_J, SDLK_L, SDLK_K, SDLK_I, SDLK_SPACE});
		ASSERT_TRUE(manager.saveMapping(*device));

		// When
		auto consumed = manager.eventUpdate(keyboardEvent(SDLK_J));

		// Then
		EXPECT_FALSE(consumed);
		EXPECT_TRUE(device->getInput().left.pressed);
	}

	TEST(DeviceManagerTest, persistenceFailureKeepsDraft) {
		// Given
		DeviceManager manager{{}, [](const std::vector<DeviceBinding>&) -> DeviceBindingsPersistenceResult {
			return std::unexpected{"disk full"};
		}};
		auto device = manager.getDefaultDevice1();
		manager.beginMapping(*device);
		captureCompleteKeyboardMapping(manager, {SDLK_J, SDLK_L, SDLK_K, SDLK_I, SDLK_SPACE});

		// When
		auto result = manager.saveMapping(*device);

		// Then
		ASSERT_FALSE(result);
		EXPECT_EQ(result.error(), "disk full");
		EXPECT_TRUE(manager.getMappingStatus(*device));
	}

	TEST(DeviceManagerTest, persistenceFailureDoesNotApplyDraft) {
		// Given
		DeviceManager manager{{}, [](const std::vector<DeviceBinding>&) -> DeviceBindingsPersistenceResult {
			return std::unexpected{"disk full"};
		}};
		auto device = manager.getDefaultDevice1();
		manager.beginMapping(*device);
		captureCompleteKeyboardMapping(manager, {SDLK_J, SDLK_L, SDLK_K, SDLK_I, SDLK_SPACE});
		ASSERT_FALSE(manager.saveMapping(*device));
		manager.cancelMapping(*device);

		// When
		manager.eventUpdate(keyboardEvent(SDLK_LEFT));

		// Then
		EXPECT_TRUE(device->getInput().left.pressed);
		EXPECT_FALSE(device->getInput().right.pressed);
	}

	TEST(DeviceManagerTest, loadsCustomMapping) {
		// Given
		KeyboardBindings custom = getDefaultKeyboardBindings(1);
		custom.left = SDLK_J;
		custom.right = SDLK_L;
		DeviceManager manager{{DeviceBinding{"Keyboard 1", custom}}};
		auto device = manager.getDefaultDevice1();

		// When
		manager.eventUpdate(keyboardEvent(SDLK_J));

		// Then
		EXPECT_TRUE(device->getInput().left.pressed);
	}

	TEST(DeviceManagerTest, resetAndSaveRestoresDefaultMapping) {
		// Given
		KeyboardBindings custom = getDefaultKeyboardBindings(1);
		custom.left = SDLK_J;
		DeviceManager manager{{DeviceBinding{"Keyboard 1", custom}}};
		auto device = manager.getDefaultDevice1();
		manager.beginMapping(*device);

		// When
		manager.resetMapping(*device);
		auto result = manager.saveMapping(*device);
		manager.tick();
		manager.eventUpdate(keyboardEvent(SDLK_LEFT));

		// Then
		ASSERT_TRUE(result);
		EXPECT_TRUE(device->getInput().left.pressed);
	}

	TEST(DeviceManagerTest, escapeCancelsMappingAndIsConsumed) {
		// Given
		DeviceManager manager;
		auto device = manager.getDefaultDevice1();
		manager.beginMapping(*device);

		// When
		EXPECT_TRUE(manager.eventUpdate(keyboardEvent(SDLK_ESCAPE)));

		// Then
		EXPECT_FALSE(manager.getMappingStatus(*device));
	}

	class GamepadDeviceManagerTest : public ::testing::Test {
	protected:
		void SetUp() override {
			ASSERT_TRUE(SDL_InitSubSystem(SDL_INIT_GAMEPAD));
			gamepadInitialized_ = true;
			SDL_INIT_INTERFACE(&description_);
			description_.type = SDL_JOYSTICK_TYPE_GAMEPAD;
			description_.vendor_id = 0x1234;
			description_.product_id = 0x5678;
			description_.nbuttons = SDL_GAMEPAD_BUTTON_COUNT;
			description_.name = "MWetris virtual gamepad";
			instanceId_ = SDL_AttachVirtualJoystick(&description_);
			ASSERT_NE(instanceId_, 0u) << SDL_GetError();
			ASSERT_TRUE(SDL_IsGamepad(instanceId_)) << SDL_GetError();

			guid_ = sdl::guidToString(SDL_GetJoystickGUIDForID(instanceId_));
			auto custom = getDefaultGamepadBindings();
			custom.left = SDL_GAMEPAD_BUTTON_WEST;
			custom.right = SDL_GAMEPAD_BUTTON_EAST;
			manager_ = std::make_unique<DeviceManager>(std::vector{DeviceBinding{guid_, custom}});
			manager_->eventUpdate(gamepadDeviceEvent(instanceId_, SDL_EVENT_GAMEPAD_ADDED));
			device_ = manager_->findDevice(guid_);
			ASSERT_TRUE(device_);
			ASSERT_TRUE(device_->isConnected());
		}

		void TearDown() override {
			if (instanceId_ != 0) {
				SDL_DetachVirtualJoystick(instanceId_);
			}
			manager_.reset();
			if (gamepadInitialized_) {
				SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
			}
		}

		bool disconnectGamepad() {
			auto removedId = instanceId_;
			if (!SDL_DetachVirtualJoystick(removedId)) {
				return false;
			}
			instanceId_ = 0;
			manager_->eventUpdate(gamepadDeviceEvent(removedId, SDL_EVENT_GAMEPAD_REMOVED));
			return true;
		}

		bool captureDefaultMapping() {
			const std::array buttons{
				SDL_GAMEPAD_BUTTON_DPAD_LEFT,
				SDL_GAMEPAD_BUTTON_DPAD_RIGHT,
				SDL_GAMEPAD_BUTTON_DPAD_DOWN,
				SDL_GAMEPAD_BUTTON_NORTH,
				SDL_GAMEPAD_BUTTON_SOUTH,
			};
			bool consumed = true;
			for (auto button : buttons) {
				consumed = manager_->eventUpdate(gamepadButtonEvent(instanceId_, button)) && consumed;
			}
			return consumed;
		}

		bool gamepadInitialized_ = false;
		SDL_VirtualJoystickDesc description_{};
		SDL_JoystickID instanceId_ = 0;
		std::string guid_;
		std::unique_ptr<DeviceManager> manager_;
		DevicePtr device_;
	};

	TEST_F(GamepadDeviceManagerTest, usesConfiguredMapping) {
		// Given
		ASSERT_TRUE(device_->isConnected());

		// When
		manager_->eventUpdate(gamepadButtonEvent(instanceId_, SDL_GAMEPAD_BUTTON_WEST));

		// Then
		EXPECT_TRUE(device_->getInput().left.pressed);
	}

	TEST_F(GamepadDeviceManagerTest, capturesMappingWithoutGameplayInput) {
		// Given
		manager_->beginMapping(*device_);

		// When
		auto consumed = captureDefaultMapping();

		// Then
		EXPECT_TRUE(consumed);
		ASSERT_TRUE(manager_->getMappingStatus(*device_));
		EXPECT_EQ(manager_->getMappingStatus(*device_)->completed, 5);
		EXPECT_EQ(device_->getInput(), Input{});
	}

	TEST_F(GamepadDeviceManagerTest, marksDisconnectedGamepadUnavailable) {
		// Given
		ASSERT_TRUE(device_->isConnected());

		// When
		auto detached = disconnectGamepad();

		// Then
		ASSERT_TRUE(detached);
		EXPECT_FALSE(device_->isConnected());
	}

	TEST_F(GamepadDeviceManagerTest, reusesDeviceAndMappingAfterReconnect) {
		// Given
		manager_->beginMapping(*device_);
		ASSERT_TRUE(captureDefaultMapping());
		ASSERT_TRUE(manager_->saveMapping(*device_));
		ASSERT_TRUE(disconnectGamepad());

		// When
		instanceId_ = SDL_AttachVirtualJoystick(&description_);
		ASSERT_NE(instanceId_, 0u) << SDL_GetError();
		manager_->eventUpdate(gamepadDeviceEvent(instanceId_, SDL_EVENT_GAMEPAD_ADDED));
		manager_->eventUpdate(gamepadButtonEvent(instanceId_, SDL_GAMEPAD_BUTTON_DPAD_LEFT));

		// Then
		EXPECT_EQ(manager_->findDevice(guid_), device_);
		EXPECT_TRUE(device_->isConnected());
		EXPECT_TRUE(device_->getInput().left.pressed);
	}

}
