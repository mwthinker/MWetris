#include "gamepad.h"

#include <SDL3/SDL.h>

#include <algorithm>

namespace app::game {

	namespace {

		void handleButton(bool& held, bool isDown) {
			held = isDown;
		}

	}

	GamePad::GamePad(sdl::GameController&& gameController, GamepadBindings bindings)
		: gameController_{std::move(gameController)}
		, bindings_{bindings}
		, defaults_{getDefaultGamepadBindings()} {
	}

	Input GamePad::getInput() const {
		return input_;
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

	bool GamePad::eventUpdate(const SDL_Event& windowEvent, std::optional<InputCommand>& command) {
		command.reset();
		if ((windowEvent.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN || windowEvent.type == SDL_EVENT_GAMEPAD_BUTTON_UP)
			&& gameController_.isAttached()
			&& windowEvent.gbutton.which == gameController_.getInstanceId()) {
			auto action = updateInput(
				static_cast<SDL_GamepadButton>(windowEvent.gbutton.button),
				windowEvent.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN);
			if (action && windowEvent.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
				command = InputCommand{*action, windowEvent.common.timestamp};
			}
			return action.has_value();
		}
		return false;
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
		clearInput();
		return true;
	}

	void GamePad::resetInput() {
		clearInput();
	}

	void GamePad::setGameController(sdl::GameController&& gameController) {
		gameController_ = std::move(gameController);
		clearInput();
	}

	std::optional<InputAction> GamePad::updateInput(SDL_GamepadButton button, bool state) {
		if (button == bindings_.left) {
			handleButton(input_.left, state);
			return InputAction::Left;
		} else if (button == bindings_.right) {
			handleButton(input_.right, state);
			return InputAction::Right;
		} else if (button == bindings_.softDrop) {
			handleButton(input_.down, state);
			return InputAction::SoftDrop;
		} else if (button == bindings_.rotate) {
			handleButton(input_.rotate, state);
			return InputAction::Rotate;
		} else if (button == bindings_.hardDrop) {
			handleButton(input_.downGround, state);
			return InputAction::HardDrop;
		} else {
			return std::nullopt;
		}
	}

	void GamePad::beginMapping() {
		draft_ = bindings_;
		mappingIndex_ = 0;
		clearInput();
	}

	void GamePad::resetMapping() {
		if (draft_) {
			draft_ = defaults_;
			mappingIndex_ = static_cast<int>(InputActions.size());
			clearInput();
		}
	}

	void GamePad::cancelMapping() {
		draft_.reset();
		mappingIndex_ = 0;
		clearInput();
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

	void GamePad::clearInput() {
		input_ = {};
	}

}
