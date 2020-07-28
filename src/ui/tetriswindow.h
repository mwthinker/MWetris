#ifndef TETRIS_UI_TETRISWINDOW_H
#define TETRIS_UI_TETRISWINDOW_H

#include "scene/scene.h"
#include "scene/menu.h"
#include "scene/statemachine.h"
#include "scene/event.h"

#include <sdl/imguiwindow.h>

#include <imgui.h>
#include <entt/entt.hpp>

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

		void imGuiPreUpdate(const std::chrono::high_resolution_clock::duration& deltaTime) override;

		void imGuiUpdate(const std::chrono::high_resolution_clock::duration& deltaTime) override;

		void imGuiPostUpdate(const std::chrono::high_resolution_clock::duration& deltaTime) override;

		void eventUpdate(const SDL_Event& windowEvent) override;

		void handleSceneMenuEvent(const scene::Event& menuEvent);
	
		std::shared_ptr<entt::dispatcher> dispatcher_{};
		sdl::Sprite background_;
		scene::StateMachine sceneStateMachine_;
	};

}

#endif
