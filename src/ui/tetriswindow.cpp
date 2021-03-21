#include "tetriswindow.h"
#include "imguiextra.h"
#include "configuration.h"

#include "scene/menu.h"
#include "scene/play.h"
#include "scene/network.h"
#include "scene/settings.h"
#include "scene/highscore.h"
#include "scene/custom.h"

#include <sdl/imguiauxiliary.h>

#include <spdlog/spdlog.h>

namespace mwetris::ui {

	TetrisWindow::TetrisWindow() {
		setPosition(mwetris::Configuration::getInstance().getWindowPositionX(), mwetris::Configuration::getInstance().getWindowPositionY());
		setSize(mwetris::Configuration::getInstance().getWindowWidth(), mwetris::Configuration::getInstance().getWindowHeight());
		setResizeable(mwetris::Configuration::getInstance().getWindowWidth());
		setTitle("MWetris");
		setIcon(mwetris::Configuration::getInstance().getWindowIcon());
		setBordered(mwetris::Configuration::getInstance().isWindowBordered());
		setShowDemoWindow(true);
		
		sceneStateMachine_.setCallback([this](scene::Event event) {
			handleSceneMenuEvent(event);
		});
	}

	TetrisWindow::~TetrisWindow() {
	}

	void TetrisWindow::initPreLoop() {
		sdl::ImGuiWindow::initPreLoop();
		auto& io{ImGui::GetIO()};
		
		mwetris::Configuration::getInstance().bindTextureFromAtlas();
		background_ = mwetris::Configuration::getInstance().getBackgroundSprite();

		io.Fonts->AddFontFromFileTTF("fonts/Ubuntu-B.ttf", 12);
		Configuration::getInstance().getImGuiButtonFont();
		Configuration::getInstance().getImGuiDefaultFont();
		Configuration::getInstance().getImGuiHeaderFont();

		sceneStateMachine_.emplace<scene::Menu>();
		sceneStateMachine_.emplace<scene::Play>(graphic_);
		sceneStateMachine_.emplace<scene::Network>();
		sceneStateMachine_.emplace<scene::Settings>();
		sceneStateMachine_.emplace<scene::HighScore>();

		shader_ = sdl::Shader::CreateShaderGlsl_330();
	}

	void TetrisWindow::imGuiPreUpdate(const sdl::DeltaTime& deltaTime) {
		shader_.useProgram();
		graphic_.clearDraw();
		graphic_.addRectangleImage({-1, -1}, {2, 2}, background_);
		graphic_.draw(shader_);
		sceneStateMachine_.draw(shader_, deltaTime);
	}
	
	void TetrisWindow::imGuiUpdate(const sdl::DeltaTime& deltaTime) {
		ImGui::MainWindow("Main", [&]() {
			sceneStateMachine_.imGuiUpdate(deltaTime);
		});
	}

	void TetrisWindow::imGuiPostUpdate(const sdl::DeltaTime& deltaTime) {
	}

	void TetrisWindow::imGuiEventUpdate(const SDL_Event& windowEvent) {
		switch (windowEvent.type) {
			case SDL_WINDOWEVENT:
				switch (windowEvent.window.event) {
					case SDL_WINDOWEVENT_RESIZED:
						glViewport(0, 0, windowEvent.window.data1, windowEvent.window.data2);
						break;
					case SDL_WINDOWEVENT_LEAVE:
						break;
					case SDL_WINDOWEVENT_CLOSE:
						sdl::Window::quit();
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
		sceneStateMachine_.eventUpdate(windowEvent);
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

	SDL_HitTestResult TetrisWindow::onHitTest(const SDL_Point& point) {
		return SDL_HITTEST_NORMAL;
	}

}
