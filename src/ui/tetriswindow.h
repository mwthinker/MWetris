#ifndef TETRIS_UI_TETRISWINDOW_H
#define TETRIS_UI_TETRISWINDOW_H

#include "../game/tetrisgame.h"
#include "../graphic/graphic.h"
#include "../graphic/gamecomponent.h"
#include "../graphic/drawboard.h"
#include "scene/scene.h"
#include "scene/menu.h"
#include "scene/statemachine.h"
#include "scene/event.h"

#include "../game/device.h"
#include "../game/sdldevice.h"

#include <sdl/imguiwindow.h>
#include <sdl/sprite.h>
#include <sdl/textureatlas.h>

#include <imgui.h>
#include <array>
#include <entt/entt.hpp>

namespace tetris {

	class GameComponent;

}

namespace tetris::ui {

	class TetrisWindow : public sdl::ImGuiWindow {
	public:
		TetrisWindow();

		~TetrisWindow();

		void setStartPage(scene::Event event) {
			handleSceneMenuEvent(event);
		}
	
	private:
		void initPreLoop() override;
	
		std::vector<game::DevicePtr> getCurrentDevices() const;
		game::DevicePtr findHumanDevice(std::string name) const;
		game::DevicePtr findAiDevice(std::string name) const;
		void resumeGame();

		ImFontAtlas fontAtlas_;
		sdl::Sprite manTexture_;
		sdl::Sprite noManTexture_;
		sdl::Sprite aiTexture_;
		sdl::Sprite background_;
		ImColor labelColor_;
		ImColor buttonTextColor_;

		void changePage(scene::Event event);
		
		int nbrHumans_;
		int nbrAis_;

	private:
		void imGuiPreUpdate(const std::chrono::high_resolution_clock::duration& deltaTime) override;

		void imGuiUpdate(const std::chrono::high_resolution_clock::duration& deltaTime) override;

		void imGuiPostUpdate(const std::chrono::high_resolution_clock::duration& deltaTime) override;

		void eventUpdate(const SDL_Event& windowEvent) override;

		void handleSceneMenuEvent(const scene::Event& menuEvent);
	
		std::shared_ptr<entt::dispatcher> dispatcher_{};
		float menuHeight_{};
		game::TetrisGame game_;
		std::array<game::DevicePtr, 4> activeAis_;
		std::unique_ptr<graphic::GameComponent> gameComponent_;
		graphic::Graphic graphic;
		std::vector<game::SdlDevicePtr> devices_;
		scene::StateMachine sceneStateMachine_;

		std::shared_ptr<scene::Scene> menuScene_;
		std::shared_ptr<scene::Scene> playScene_;
		std::shared_ptr<scene::Scene> networkScene_;
		std::shared_ptr<scene::Scene> settingsScene_;
		std::shared_ptr<scene::Scene> highscoreScene_;
		std::shared_ptr<scene::Scene> customScene_;
	};

}

#endif
