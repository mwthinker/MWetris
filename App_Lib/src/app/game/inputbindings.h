#ifndef APP_GAME_INPUTBINDINGS_H
#define APP_GAME_INPUTBINDINGS_H

#include <SDL3/SDL.h>

#include <array>
#include <concepts>
#include <expected>
#include <string>
#include <type_traits>
#include <variant>

namespace app::game {

	enum class InputAction {
		Left,
		Right,
		SoftDrop,
		Rotate,
		HardDrop,
	};

	enum class InputKind {
		KeyboardKey,
		GamepadButton,
	};

	struct MappingStatus {
		InputAction awaiting = InputAction::Left;
		InputKind acceptedInput = InputKind::KeyboardKey;
		int completed = 0;
		int total = 5;
	};

	inline constexpr std::array InputActions{
		InputAction::Left,
		InputAction::Right,
		InputAction::SoftDrop,
		InputAction::Rotate,
		InputAction::HardDrop,
	};

	struct KeyboardBindings {
		SDL_Keycode left = SDLK_LEFT;
		SDL_Keycode right = SDLK_RIGHT;
		SDL_Keycode softDrop = SDLK_DOWN;
		SDL_Keycode rotate = SDLK_UP;
		SDL_Keycode hardDrop = SDLK_RCTRL;

		bool operator==(const KeyboardBindings&) const = default;
	};

	struct GamepadBindings {
		SDL_GamepadButton left = SDL_GAMEPAD_BUTTON_DPAD_LEFT;
		SDL_GamepadButton right = SDL_GAMEPAD_BUTTON_DPAD_RIGHT;
		SDL_GamepadButton softDrop = SDL_GAMEPAD_BUTTON_DPAD_DOWN;
		SDL_GamepadButton rotate = SDL_GAMEPAD_BUTTON_DPAD_UP;
		SDL_GamepadButton hardDrop = SDL_GAMEPAD_BUTTON_SOUTH;

		bool operator==(const GamepadBindings&) const = default;
	};

	template<class T>
	concept Bindings = requires(T bindings, const T constBindings) {
		bindings.left;
		bindings.right;
		bindings.softDrop;
		bindings.rotate;
		bindings.hardDrop;

		requires std::same_as<
			std::remove_cvref_t<decltype(bindings.left)>,
			std::remove_cvref_t<decltype(bindings.right)>>;
		requires std::same_as<
			std::remove_cvref_t<decltype(bindings.left)>,
			std::remove_cvref_t<decltype(bindings.softDrop)>>;
		requires std::same_as<
			std::remove_cvref_t<decltype(bindings.left)>,
			std::remove_cvref_t<decltype(bindings.rotate)>>;
		requires std::same_as<
			std::remove_cvref_t<decltype(bindings.left)>,
			std::remove_cvref_t<decltype(bindings.hardDrop)>>;

		{ constBindings == constBindings } -> std::convertible_to<bool>;
	};

	using BindingSet = std::variant<KeyboardBindings, GamepadBindings>;
	using BindingsValidationResult = std::expected<void, std::string>;

	struct DeviceBinding {
		std::string deviceGuid;
		BindingSet bindings;

		bool operator==(const DeviceBinding&) const = default;
	};

	using DeviceBindingsPersistenceResult = std::expected<void, std::string>;

	inline constexpr const char* getInputActionName(InputAction action) {
		switch (action) {
			case InputAction::Left: return "Left";
			case InputAction::Right: return "Right";
			case InputAction::SoftDrop: return "Soft drop";
			case InputAction::Rotate: return "Rotate";
			case InputAction::HardDrop: return "Hard drop";
		}
		return "Unknown";
	}

	inline constexpr KeyboardBindings getDefaultKeyboardBindings(int keyboardNumber) {
		if (keyboardNumber == 2) {
			return {
				.left = SDLK_A,
				.right = SDLK_D,
				.softDrop = SDLK_S,
				.rotate = SDLK_W,
				.hardDrop = SDLK_LCTRL,
			};
		}
		return {};
	}

	inline constexpr GamepadBindings getDefaultGamepadBindings() {
		return {};
	}

	constexpr auto getBinding(const Bindings auto& bindings, InputAction action) {
		switch (action) {
			case InputAction::Left: return bindings.left;
			case InputAction::Right: return bindings.right;
			case InputAction::SoftDrop: return bindings.softDrop;
			case InputAction::Rotate: return bindings.rotate;
			case InputAction::HardDrop: return bindings.hardDrop;
		}
		return bindings.left;
	}

	constexpr void setBinding(Bindings auto& bindings, InputAction action, auto value) {
		switch (action) {
			case InputAction::Left: bindings.left = value; break;
			case InputAction::Right: bindings.right = value; break;
			case InputAction::SoftDrop: bindings.softDrop = value; break;
			case InputAction::Rotate: bindings.rotate = value; break;
			case InputAction::HardDrop: bindings.hardDrop = value; break;
		}
	}

	constexpr void assignBindingWithSwap(Bindings auto& bindings, InputAction action, auto value) {
		const auto previous = getBinding(bindings, action);
		for (auto candidate : InputActions) {
			if (candidate != action && getBinding(bindings, candidate) == value) {
				setBinding(bindings, candidate, previous);
				break;
			}
		}
		setBinding(bindings, action, value);
	}

	BindingsValidationResult validateUniqueBindings(const Bindings auto& bindings) {
		for (int first = 0; first < std::ssize(InputActions); ++first) {
			for (int second = first + 1; second < std::ssize(InputActions); ++second) {
				if (getBinding(bindings, InputActions[first]) == getBinding(bindings, InputActions[second])) {
					return std::unexpected{"Every action must use a different input."};
				}
			}
		}
		return {};
	}

	inline BindingsValidationResult validateBindings(const KeyboardBindings& bindings) {
		for (auto action : InputActions) {
			auto key = getBinding(bindings, action);
			if (key == SDLK_UNKNOWN || key == SDLK_ESCAPE) {
				return std::unexpected{"Bindings cannot use Unknown or Escape."};
			}
		}
		return validateUniqueBindings(bindings);
	}

	inline BindingsValidationResult validateBindings(const GamepadBindings& bindings) {
		for (auto action : InputActions) {
			auto button = getBinding(bindings, action);
			if (button <= SDL_GAMEPAD_BUTTON_INVALID || button >= SDL_GAMEPAD_BUTTON_COUNT) {
				return std::unexpected{"A gamepad binding contains an invalid button."};
			}
		}
		return validateUniqueBindings(bindings);
	}

}

#endif
