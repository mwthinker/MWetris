#include "tetriswindow.h"
#include "imguiextra.h"
#include "tetrisdata.h"

#include "scene/menu.h"
#include "scene/play.h"
#include "scene/network.h"
#include "scene/settings.h"
#include "scene/highscore.h"
#include "scene/custom.h"

#include <sdl/imguiauxiliary.h>

#include <spdlog/spdlog.h>

namespace {

	constexpr ImGuiWindowFlags ImGuiNoWindow = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMove;

}

namespace mwetris::ui {

	TetrisWindow::TetrisWindow() {
		background_ = mwetris::TetrisData::getInstance().getBackgroundSprite();

		setPosition(mwetris::TetrisData::getInstance().getWindowPositionX(), mwetris::TetrisData::getInstance().getWindowPositionY());
		setSize(mwetris::TetrisData::getInstance().getWindowWidth(), mwetris::TetrisData::getInstance().getWindowHeight());
		setResizeable(mwetris::TetrisData::getInstance().getWindowWidth());
		setTitle("MWetris");
		setIcon(mwetris::TetrisData::getInstance().getWindowIcon());
		setBordered(mwetris::TetrisData::getInstance().isWindowBordered());
		setShowDemoWindow(true);
		//ImGui::GetStyle().

		//ImGui::PushStyleColor(ImGuiCol_Button, buttonTextColor_.Value);
		
		sceneStateMachine_.setCallback([this](scene::Event event) {
			handleSceneMenuEvent(event);
		});
	}

	TetrisWindow::~TetrisWindow() {
	}

	void TetrisWindow::initPreLoop() {
		sdl::ImGuiWindow::initPreLoop();
		auto& io{ImGui::GetIO()};

		background_.bindTexture();
		mwetris::TetrisData::getInstance().bindTextureFromAtlas();
		//ImGui::GetStyle().WindowBorderSize = 0;

		TetrisData::getInstance().getImGuiButtonFont();
		TetrisData::getInstance().getImGuiDefaultFont();
		TetrisData::getInstance().getImGuiHeaderFont();

		sceneStateMachine_.emplace<scene::Menu>();
		sceneStateMachine_.emplace<scene::Play>(graphic_);
		sceneStateMachine_.emplace<scene::Network>();
		sceneStateMachine_.emplace<scene::Settings>();
		sceneStateMachine_.emplace<scene::HighScore>();

		shader_ = sdl::Shader::CreateShaderGlsl_330();
	}

	void TetrisWindow::imGuiPreUpdate(const std::chrono::high_resolution_clock::duration& deltaTime) {
		shader_.useProgram();
		graphic_.clearDraw();
		graphic_.addRectangleImage({-1, -1}, {2, 2}, background_.getTextureView());
		graphic_.draw(shader_);
		sceneStateMachine_.draw(shader_, deltaTime);
	}
	
	void TetrisWindow::imGuiUpdate(const std::chrono::high_resolution_clock::duration& deltaTime) {
		ImGui::PushFont(mwetris::TetrisData::getInstance().getImGuiDefaultFont());
		ImGui::PopFont();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, {0, 0, 0, 0});

		ImGui::SetNextWindowPos({0.f,0.f});
		auto [width, height] = sdl::Window::getSize();
		ImGui::SetNextWindowSize({static_cast<float>(width), static_cast<float>(height)});
		
		ImGui::Window("Main", nullptr, ImGuiNoWindow, [&]() {
			sceneStateMachine_.imGuiUpdate(deltaTime);
		});
		ImGui::PopStyleColor();
		ImGui::PopStyleVar(3);
	}

	void TetrisWindow::imGuiPostUpdate(const std::chrono::high_resolution_clock::duration& deltaTime) {
	}

	void TetrisWindow::eventUpdate(const SDL_Event& windowEvent) {
		sdl::ImGuiWindow::eventUpdate(windowEvent);

		auto& io = ImGui::GetIO();

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
			case SDL_MOUSEBUTTONUP: [[fallthrough]];
			case SDL_MOUSEBUTTONDOWN: [[fallthrough]];
			case SDL_MOUSEMOTION: [[fallthrough]];
			case SDL_MOUSEWHEEL:
				if (!io.WantCaptureMouse) {
					sceneStateMachine_.eventUpdate(windowEvent);
				}
				break;
			case SDL_KEYUP:
				if (!io.WantCaptureKeyboard) {
					sceneStateMachine_.eventUpdate(windowEvent);
				}
				break;
			case SDL_KEYDOWN:
				if (!io.WantCaptureKeyboard) {
					sceneStateMachine_.eventUpdate(windowEvent);
					switch (windowEvent.key.keysym.sym) {
						case SDLK_ESCAPE:
							sdl::Window::quit();
							break;
					}
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
