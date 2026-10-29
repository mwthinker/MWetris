#ifndef APP_GAME_GAMEPAD_H
#define APP_GAME_GAMEPAD_H

#include "input.h"
#include "inputbindings.h"

#include <sdl/gamecontroller.h>

#include <SDL3/SDL_events.h>

#include <array>
#include <optional>
#include <string>

namespace app::game {

	class GamePad {
	public:
		GamePad(sdl::GameController&& gameController, GamepadBindings bindings = getDefaultGamepadBindings());

		const char* getName() const;
		int getInstanceId() const;
		const std::string& getGuid() const;

		std::optional<InputEvent> eventUpdate(const SDL_Event& windowEvent) const;
		bool captureMappingEvent(const SDL_Event& windowEvent);

		void beginMapping();
		void resetMapping();
		void cancelMapping();
		BindingsValidationResult commitMapping();
		std::optional<MappingStatus> getMappingStatus() const;
		std::array<std::string, 5> getMappingLabels() const;
		const GamepadBindings& getBindings() const;
		std::optional<GamepadBindings> getDraftBindings() const;

		void setGameController(sdl::GameController&& gameController);

	private:
		std::optional<InputAction> getAction(SDL_GamepadButton button) const;

		sdl::GameController gameController_;
		GamepadBindings bindings_{};
		GamepadBindings defaults_{};
		std::optional<GamepadBindings> draft_;
		int mappingIndex_ = 0;
		mutable std::string guid_;
	};

}

#endif
