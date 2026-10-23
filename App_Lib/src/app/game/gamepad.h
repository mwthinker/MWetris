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

		Input getInput() const;
		const char* getName() const;
		int getInstanceId() const;
		const std::string& getGuid() const;

		bool eventUpdate(const SDL_Event& windowEvent);
		bool captureMappingEvent(const SDL_Event& windowEvent);
		void tick();

		void beginMapping();
		void resetMapping();
		void cancelMapping();
		BindingsValidationResult commitMapping();
		std::optional<MappingStatus> getMappingStatus() const;
		std::array<std::string, 5> getMappingLabels() const;
		const GamepadBindings& getBindings() const;
		std::optional<GamepadBindings> getDraftBindings() const;

		bool isActive() const {
			return true;
		}

		void setGameController(sdl::GameController&& gameController);

	private:
		bool updateInput(SDL_GamepadButton button, bool state);
		void clearInput();

		Input input_{};
		sdl::GameController gameController_;
		GamepadBindings bindings_{};
		GamepadBindings defaults_{};
		std::optional<GamepadBindings> draft_;
		int mappingIndex_ = 0;
		mutable std::string guid_;
	};

}

#endif
