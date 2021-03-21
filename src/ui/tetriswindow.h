#ifndef MWETRIS_UI_TETRISWINDOW_H
#define MWETRIS_UI_TETRISWINDOW_H

#include "scene/scene.h"
#include "scene/menu.h"
#include "scene/statemachine.h"
#include "scene/event.h"

#include <sdl/imguiwindow.h>
#include <sdl/shader.h>

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

		void imGuiPreUpdate(const sdl::DeltaTime& deltaTime) override;

		void imGuiUpdate(const sdl::DeltaTime& deltaTime) override;

		void imGuiPostUpdate(const sdl::DeltaTime& deltaTime) override;

		void imGuiEventUpdate(const SDL_Event& windowEvent) override;

		void handleSceneMenuEvent(const scene::Event& menuEvent);

		SDL_HitTestResult onHitTest(const SDL_Point& point) override;
	
		sdl::TextureView background_;
		scene::StateMachine sceneStateMachine_;
		sdl::Shader shader_;
		graphic::Graphic graphic_;
	};

}

#endif
