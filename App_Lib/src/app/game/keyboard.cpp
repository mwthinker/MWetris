#include "keyboard.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <string>

namespace app::game {

	namespace {

		void handleKey(KeyState& keyState, bool isDown) {
			if (isDown) {
				if (!keyState.held) {
					keyState.pressed = true;
				}
				keyState.held = true;
			} else {
				if (keyState.held) {
					keyState.released = true;
				}
				keyState.held = false;
			}
		}

	}

	void Keyboard::tick() {
		input_.left.pressed = false;
		input_.left.released = false;
		input_.right.pressed = false;
		input_.right.released = false;
		input_.down.pressed = false;
		input_.down.released = false;
		input_.rotate.pressed = false;
		input_.rotate.released = false;
		input_.downGround.pressed = false;
		input_.downGround.released = false;
	}

	Keyboard::Keyboard(const std::string& name, const KeyboardMapping& keyboardMapping)
		: name_{name}
		, bindings_{keyboardMapping}
		, defaults_{getDefaultKeyboardBindings(name == "Keyboard 2" ? 2 : 1)} {
	}

	Input Keyboard::getInput() const {
		return input_;
	}

	const char* Keyboard::getName() const {
		return name_.c_str();
	}

	bool Keyboard::eventUpdate(const SDL_Event& windowEvent) {
		if (windowEvent.type != SDL_EVENT_KEY_DOWN && windowEvent.type != SDL_EVENT_KEY_UP) {
			return false;
		}
		if (windowEvent.key.repeat != 0) {
			return false;
		}

		const bool isDown = windowEvent.type == SDL_EVENT_KEY_DOWN;
		const auto key = windowEvent.key.key;
		if (key == bindings_.left) {
			handleKey(input_.left, isDown);
		} else if (key == bindings_.right) {
			handleKey(input_.right, isDown);
		} else if (key == bindings_.softDrop) {
			handleKey(input_.down, isDown);
		} else if (key == bindings_.rotate) {
			handleKey(input_.rotate, isDown);
		} else if (key == bindings_.hardDrop) {
			handleKey(input_.downGround, isDown);
		}
		return true;
	}

	bool Keyboard::captureMappingEvent(const SDL_Event& windowEvent) {
		if (!draft_ || (windowEvent.type != SDL_EVENT_KEY_DOWN && windowEvent.type != SDL_EVENT_KEY_UP)) {
			return false;
		}
		if (windowEvent.key.scancode > SDL_SCANCODE_UNKNOWN && windowEvent.key.scancode < SDL_SCANCODE_COUNT) {
			mappingKeyState_[windowEvent.key.scancode] = windowEvent.type == SDL_EVENT_KEY_DOWN;
		}
		if (windowEvent.type == SDL_EVENT_KEY_UP || windowEvent.key.repeat != 0) {
			return true;
		}
		if (windowEvent.key.key == SDLK_ESCAPE || mappingIndex_ >= std::ssize(InputActions)) {
			return true;
		}
		assignBindingWithSwap(*draft_, InputActions[mappingIndex_], windowEvent.key.key);
		++mappingIndex_;
		clearInput();
		return true;
	}

	MappingPollResult Keyboard::pollMappingInput() {
		if (!draft_) {
			return MappingPollResult::None;
		}
		int keyCount = 0;
		const auto* keyState = SDL_GetKeyboardState(&keyCount);
		const int availableKeys = std::min(keyCount, static_cast<int>(SDL_SCANCODE_COUNT));
		SDL_Scancode pressedScancode = SDL_SCANCODE_UNKNOWN;
		for (int index = 1; index < availableKeys; ++index) {
			if (keyState[index] && !mappingKeyState_[index] && pressedScancode == SDL_SCANCODE_UNKNOWN) {
				pressedScancode = static_cast<SDL_Scancode>(index);
			}
			mappingKeyState_[index] = keyState[index];
		}
		if (pressedScancode == SDL_SCANCODE_UNKNOWN) {
			return MappingPollResult::None;
		}
		auto key = SDL_GetKeyFromScancode(pressedScancode, SDL_KMOD_NONE, false);
		if (key == SDLK_ESCAPE) {
			return MappingPollResult::CancelRequested;
		}
		if (key == SDLK_UNKNOWN || mappingIndex_ >= std::ssize(InputActions)) {
			return MappingPollResult::None;
		}
		assignBindingWithSwap(*draft_, InputActions[mappingIndex_], key);
		++mappingIndex_;
		clearInput();
		return MappingPollResult::Captured;
	}

	void Keyboard::beginMapping() {
		draft_ = bindings_;
		mappingIndex_ = 0;
		int keyCount = 0;
		const auto* keyState = SDL_GetKeyboardState(&keyCount);
		mappingKeyState_.fill(false);
		std::copy_n(keyState, std::min(keyCount, static_cast<int>(SDL_SCANCODE_COUNT)), mappingKeyState_.begin());
		clearInput();
	}

	void Keyboard::resetMapping() {
		if (draft_) {
			draft_ = defaults_;
			mappingIndex_ = static_cast<int>(InputActions.size());
			clearInput();
		}
	}

	void Keyboard::cancelMapping() {
		draft_.reset();
		mappingIndex_ = 0;
		clearInput();
	}

	BindingsValidationResult Keyboard::commitMapping() {
		if (!draft_ || mappingIndex_ < std::ssize(InputActions)) {
			return std::unexpected{"Capture all five actions before saving."};
		}
		if (auto validation = validateBindings(*draft_); !validation) {
			return validation;
		}
		bindings_ = *draft_;
		cancelMapping();
		return {};
	}

	std::optional<MappingStatus> Keyboard::getMappingStatus() const {
		if (!draft_) {
			return std::nullopt;
		}
		return MappingStatus{
			.awaiting = InputActions[std::min(mappingIndex_, static_cast<int>(InputActions.size()) - 1)],
			.acceptedInput = InputKind::KeyboardKey,
			.completed = mappingIndex_,
			.total = static_cast<int>(InputActions.size()),
		};
	}

	std::array<std::string, 5> Keyboard::getMappingLabels() const {
		const auto& source = draft_ ? *draft_ : bindings_;
		std::array<std::string, 5> labels;
		for (int index = 0; index < std::ssize(InputActions); ++index) {
			labels[index] = SDL_GetKeyName(getBinding(source, InputActions[index]));
		}
		return labels;
	}

	const KeyboardBindings& Keyboard::getBindings() const {
		return bindings_;
	}

	std::optional<KeyboardBindings> Keyboard::getDraftBindings() const {
		return draft_;
	}

	void Keyboard::clearInput() {
		input_ = {};
	}

}
