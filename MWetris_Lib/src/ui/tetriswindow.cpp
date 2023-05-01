#include "tetriswindow.h"
#include "imguiextra.h"
#include "configuration.h"

#include "scene/menu.h"
#include "scene/play.h"
#include "scene/network.h"
#include "scene/settings.h"
#include "scene/highscore.h"

#include <sdl/imguiauxiliary.h>

#include <spdlog/spdlog.h>

namespace mwetris::ui {

	TetrisWindow::TetrisWindow() {
		setPosition(Configuration::getInstance().getWindowPositionX(), Configuration::getInstance().getWindowPositionY());
		setSize(Configuration::getInstance().getWindowWidth(), Configuration::getInstance().getWindowHeight());
		setResizeable(Configuration::getInstance().getWindowWidth());
		setTitle("MWetris");
		setIcon(Configuration::getInstance().getWindowIcon());
		setBordered(Configuration::getInstance().isWindowBordered());
		setShowDemoWindow(true);
		
		sceneStateMachine_.setCallback([&](scene::Event event) {
			handleSceneMenuEvent(event);
		});
	}

	TetrisWindow::~TetrisWindow() {
		Configuration::getInstance().quit();
	}

	void TetrisWindow::initPreLoop() {
		sdl::ImGuiWindow::initPreLoop();
		auto& io{ImGui::GetIO()};
		
		Configuration::getInstance().bindTextureFromAtlas();
		background_ = Configuration::getInstance().getBackgroundSprite();
		
		io.Fonts->AddFontFromFileTTF("fonts/Ubuntu-B.ttf", 12); // Used by demo window.
		Configuration::getInstance().getImGuiButtonFont();
		Configuration::getInstance().getImGuiDefaultFont();
		Configuration::getInstance().getImGuiHeaderFont();
				
		sceneStateMachine_.emplace<scene::Menu>();
		sceneStateMachine_.emplace<scene::Play>();
		sceneStateMachine_.emplace<scene::Network>();
		sceneStateMachine_.emplace<scene::Settings>();
		sceneStateMachine_.emplace<scene::HighScore>();
		
		handleSceneMenuEvent(startEvent_);
	}
	
	void TetrisWindow::imGuiUpdate(const sdl::DeltaTime& deltaTime) {
		ImGui::PushFont(Configuration::getInstance().getImGuiDefaultFont());
		ImGui::MainWindow("Main", [&]() {
			ImGui::ImageBackground(background_);
			sceneStateMachine_.imGuiUpdate(deltaTime);
		});
		ImGui::PopFont();
	}

	void TetrisWindow::imGuiEventUpdate(const SDL_Event& windowEvent) {
		if (!sceneStateMachine_.eventUpdate(windowEvent)) {
			return; // Don't bubble up.
		}

		switch (windowEvent.type) {
			case SDL_WINDOWEVENT:
				switch (windowEvent.window.event) {
					case SDL_WINDOWEVENT_RESIZED:
						gl::glViewport(0, 0, windowEvent.window.data1, windowEvent.window.data2);
						break;
					case SDL_WINDOWEVENT_LEAVE:
						break;
					case SDL_WINDOWEVENT_CLOSE:
						sdl::Window::quit();
						break;
				}
				break;
			case SDL_KEYDOWN:
				switch (windowEvent.key.keysym.sym) {
					case SDLK_ESCAPE:
						sdl::Window::quit();
						break;
				}
				break;
			case SDL_QUIT:
				sdl::Window::quit();
				break;
		}
	}

	void TetrisWindow::handleSceneMenuEvent(const scene::Event& menuEvent) {
		switch (menuEvent) {
			case scene::Event::Menu:
				sceneStateMachine_.switchTo<scene::Menu>();
				break;
			case scene::Event::Play:
				sceneStateMachine_.switchTo<scene::Play>();
				break;
			case scene::Event::NetworkPlay:
				sceneStateMachine_.switchTo<scene::Network>();
				break;
			case scene::Event::Settings:
				sceneStateMachine_.switchTo<scene::Settings>();
				break;
			case scene::Event::HighScore:
				sceneStateMachine_.switchTo<scene::HighScore>();
				break;
			case scene::Event::Exit:
				quit();
				return;
		}
	}

}
