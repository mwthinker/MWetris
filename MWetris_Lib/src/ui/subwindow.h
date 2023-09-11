#ifndef MWETRIS_UI_SUBWINDOW_H
#define MWETRIS_UI_SUBWINDOW_H

#include <sdl/imguiwindow.h>

namespace mwetris::ui {
	
	class SubWindow {
	public:
		virtual ~SubWindow() = default;

		virtual void imGuiUpdate(const sdl::DeltaTime& deltaTime) {}
		virtual void imGuiEventUpdate(const SDL_Event& windowEvent) {}
	};

}

#endif
