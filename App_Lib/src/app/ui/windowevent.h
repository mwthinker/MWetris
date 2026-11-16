#ifndef APP_UI_WINDOWEVENT_H
#define APP_UI_WINDOWEVENT_H

#include <SDL3/SDL_events.h>

namespace app::ui {

	inline Uint32 getToggleFullscreenEventType() {
		static const Uint32 eventType = SDL_RegisterEvents(1);
		return eventType;
	}

}

#endif
