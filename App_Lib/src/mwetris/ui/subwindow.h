#ifndef MWETRIS_UI_SUBWINDOW_H
#define MWETRIS_UI_SUBWINDOW_H

#include <sdl/imguiwindow.h>

#include <string>

namespace mwetris::ui {
	
	class SubWindow {
	public:
		enum class Type {
			MainWindow,
			SecondaryWindow
		};

		virtual ~SubWindow() = default;

		virtual void imGuiUpdate(const sdl::DeltaTime& deltaTime) {}
		virtual void imGuiEventUpdate(const SDL_Event& windowEvent) {}

		virtual const std::string& getName() const = 0;
		virtual Type getType() const = 0;
	};

}

#endif
