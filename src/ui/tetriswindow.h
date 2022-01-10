#ifndef MWETRIS_UI_TETRISWINDOW_H
#define MWETRIS_UI_TETRISWINDOW_H

#include "scene/scene.h"
#include "scene/menu.h"
#include "scene/statemachine.h"
#include "scene/event.h"

#include <sdl/imguiwindow.h>

namespace mwetris {

	class GameComponent;

}

namespace mwetris::ui {

	class TetrisWindow : public sdl::ImGuiWindow {
	public:
		TetrisWindow();

		~TetrisWindow();

		void setStartPage(scene::Event event) {
			handleSceneMenuEvent(event);
		}
	
	private:
		void initPreLoop() override;

		void imGuiUpdate(const sdl::DeltaTime& deltaTime) override;

		void imGuiEventUpdate(const SDL_Event& windowEvent) override;

		void handleSceneMenuEvent(const scene::Event& menuEvent);
	
		sdl::TextureView background_;
		scene::StateMachine sceneStateMachine_;
	};

}

#endif
