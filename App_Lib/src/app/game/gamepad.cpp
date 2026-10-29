#include "gamepad.h"

#include <SDL3/SDL.h>

#include <algorithm>

namespace app::game {

	GamePad::GamePad(sdl::GameController&& gameController, GamepadBindings bindings)
		: gameController_{std::move(gameController)}
		, bindings_{bindings}
		, defaults_{getDefaultGamepadBindings()} {
	}

	const char* GamePad::getName() const {
		return gameController_.getName();
	}

	int GamePad::getInstanceId() const {
		return gameController_.getInstanceId();
	}

	const std::string& GamePad::getGuid() const {
		if (guid_.empty()) {
			guid_ = sdl::guidToString(gameController_.getGuid());
		}
		return guid_;
	}

	std::optional<InputEvent> GamePad::eventUpdate(const SDL_Event& windowEvent) const {
		if ((windowEvent.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN || windowEvent.type == SDL_EVENT_GAMEPAD_BUTTON_UP)
			&& gameController_.isAttached()
			&& windowEvent.gbutton.which == gameController_.getInstanceId()) {
			auto action = getAction(static_cast<SDL_GamepadButton>(windowEvent.gbutton.button));
			if (action) {
				return InputEvent{
					.action = *action,
					.state = windowEvent.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN
						? InputState::Pressed
						: InputState::Released,
					.timestamp = windowEvent.common.timestamp,
				};
			}
		}
		return std::nullopt;
	}

	bool GamePad::captureMappingEvent(const SDL_Event& windowEvent) {
		if (!draft_ || (windowEvent.type != SDL_EVENT_GAMEPAD_BUTTON_DOWN && windowEvent.type != SDL_EVENT_GAMEPAD_BUTTON_UP)
			|| !gameController_.isAttached() || windowEvent.gbutton.which != gameController_.getInstanceId()) {
			return false;
		}
		if (windowEvent.type == SDL_EVENT_GAMEPAD_BUTTON_UP || mappingIndex_ >= std::ssize(InputActions)) {
			return true;
		}
		auto button = static_cast<SDL_GamepadButton>(windowEvent.gbutton.button);
		if (button <= SDL_GAMEPAD_BUTTON_INVALID || button >= SDL_GAMEPAD_BUTTON_COUNT) {
			return true;
		}
		assignBindingWithSwap(*draft_, InputActions[mappingIndex_], button);
		++mappingIndex_;
		return true;
	}

	void GamePad::setGameController(sdl::GameController&& gameController) {
		gameController_ = std::move(gameController);
	}

	std::optional<InputAction> GamePad::getAction(SDL_GamepadButton button) const {
		if (button == bindings_.left) {
			return InputAction::Left;
		} else if (button == bindings_.right) {
			return InputAction::Right;
		} else if (button == bindings_.softDrop) {
			return InputAction::SoftDrop;
		} else if (button == bindings_.rotate) {
			return InputAction::Rotate;
		} else if (button == bindings_.hardDrop) {
			return InputAction::HardDrop;
		} else {
			return std::nullopt;
		}
	}

	void GamePad::beginMapping() {
		draft_ = bindings_;
		mappingIndex_ = 0;
	}

	void GamePad::resetMapping() {
		if (draft_) {
			draft_ = defaults_;
			mappingIndex_ = static_cast<int>(InputActions.size());
		}
	}

	void GamePad::cancelMapping() {
		draft_.reset();
		mappingIndex_ = 0;
	}

	BindingsValidationResult GamePad::commitMapping() {
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

	std::optional<MappingStatus> GamePad::getMappingStatus() const {
		if (!draft_) {
			return std::nullopt;
		}
		return MappingStatus{
			.awaiting = InputActions[std::min(mappingIndex_, static_cast<int>(InputActions.size()) - 1)],
			.acceptedInput = InputKind::GamepadButton,
			.completed = mappingIndex_,
			.total = static_cast<int>(InputActions.size()),
		};
	}

	std::array<std::string, 5> GamePad::getMappingLabels() const {
		const auto& source = draft_ ? *draft_ : bindings_;
		std::array<std::string, 5> labels;
		for (int index = 0; index < std::ssize(InputActions); ++index) {
			labels[index] = SDL_GetGamepadStringForButton(getBinding(source, InputActions[index]));
		}
		return labels;
	}

	const GamepadBindings& GamePad::getBindings() const {
		return bindings_;
	}

	std::optional<GamepadBindings> GamePad::getDraftBindings() const {
		return draft_;
	}

}
