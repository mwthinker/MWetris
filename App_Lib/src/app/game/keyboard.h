#ifndef APP_GAME_KEYBOARD_H
#define APP_GAME_KEYBOARD_H

#include "input.h"
#include "inputbindings.h"

#include <SDL3/SDL.h>

#include <array>
#include <optional>
#include <string>

namespace app::game {

	using KeyboardMapping = KeyboardBindings;
	enum class MappingPollResult {
		None,
		Captured,
		CancelRequested,
	};

	class Keyboard {
	public:
		Keyboard(
			const std::string& name,
			const KeyboardMapping& keyboardMapping,
			const KeyboardMapping& defaultMapping);

		const char* getName() const;

		std::optional<InputEvent> eventUpdate(const SDL_Event& windowEvent) const;
		bool captureMappingEvent(const SDL_Event& windowEvent);
		MappingPollResult pollMappingInput();

		void beginMapping();
		void resetMapping();
		void cancelMapping();
		BindingsValidationResult commitMapping();
		std::optional<MappingStatus> getMappingStatus() const;
		std::array<std::string, 5> getMappingLabels() const;
		const KeyboardBindings& getBindings() const;
		std::optional<KeyboardBindings> getDraftBindings() const;

	private:
		std::string name_;
		KeyboardBindings bindings_{};
		KeyboardBindings defaults_{};
		std::optional<KeyboardBindings> draft_;
		std::array<bool, SDL_SCANCODE_COUNT> mappingKeyState_{};
		int mappingIndex_ = 0;
	};

}

#endif
