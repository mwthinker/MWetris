#include "tetriswindow.h"
#include "imguiextra.h"
#include "configuration.h"

#include "scene/play.h"
#include "scene/about.h"
#include "scene/network.h"
#include "scene/settings.h"
#include "scene/highscore.h"

#include "game/serialize.h"

#include <sdl/imguiauxiliary.h>

#include <spdlog/spdlog.h>

namespace mwetris::ui {

	constexpr ImGuiWindowFlags ImguiNoWindow
		= ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoBringToFrontOnFocus
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoDocking
		| ImGuiWindowFlags_NoScrollbar
		| ImGuiWindowFlags_NoBackground
		| ImGuiWindowFlags_NoScrollWithMouse
		| ImGuiWindowFlags_MenuBar;
		//| ImGuiWindowFlags_NoNavInputs;

	TetrisWindow::TetrisWindow() {
		setPosition(Configuration::getInstance().getWindowPositionX(), Configuration::getInstance().getWindowPositionY());
		setSize(Configuration::getInstance().getWindowWidth(), Configuration::getInstance().getWindowHeight());
		setResizeable(true);
		setTitle("MWetris");
		setIcon(Configuration::getInstance().getWindowIcon());
		setBordered(Configuration::getInstance().isWindowBordered());
		setShowDemoWindow(true);
	}

	TetrisWindow::~TetrisWindow() {
		Configuration::getInstance().quit();
	}

	void TetrisWindow::initPreLoop() {
		sdl::ImGuiWindow::initPreLoop();
		auto& io{ImGui::GetIO()};
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		Configuration::getInstance().bindTextureFromAtlas();
		background_ = Configuration::getInstance().getBackgroundSprite();
		
		io.Fonts->AddFontFromFileTTF("fonts/Ubuntu-B.ttf", 12); // Used by demo window.
		Configuration::getInstance().getImGuiButtonFont();
		Configuration::getInstance().getImGuiDefaultFont();
		Configuration::getInstance().getImGuiHeaderFont();
		
		sceneStateMachine_.emplace<scene::Settings>();
		sceneStateMachine_.emplace<scene::HighScore>();
		sceneStateMachine_.emplace<scene::About>();

		play_ = std::make_unique<scene::Play>();
	}

	void TetrisWindow::imGuiUpdate(const sdl::DeltaTime& deltaTime) {
		ImGui::PushFont(Configuration::getInstance().getImGuiDefaultFont());

		if (openPopUp_) {
			openPopUp_ = false;
			ImGui::OpenPopup("Popup");
		}
		ImGui::PopupModal("Popup", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar, [&]() {
			sceneStateMachine_.imGuiUpdate(deltaTime);

			if (ImGui::IsKeyDown(ImGuiKey_Escape)) {
				ImGui::CloseCurrentPopup();
			}
		});

		ImGui::MainWindow("MainWindow", ImguiNoWindow, [&]() {
			ImGui::ImageBackground(background_);
			play_->imGuiUpdate(deltaTime);

			ImGui::MenuBar([&]() {
				ImGui::Menu("Main", [&]() {
					if (ImGui::MenuItem("New Game", "F2")) {
						play_->startNewGame();
					}
					if (ImGui::MenuItem("Custom Game")) {

					}
					if (ImGui::MenuItem("Highscore")) {
						ImVec2 center = ImGui::GetMainViewport()->GetCenter();
						ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
						ImGui::SetNextWindowSize({500, 500}, ImGuiCond_Appearing);

						openPopUp<mwetris::ui::scene::HighScore>();

						results_ = game::loadHighScore();
					}
					if (ImGui::MenuItem("Quit", "ESQ")) {
						sdl::Window::quit();
					}
				});
				ImGui::Menu("Settings", [&]() {
					if (ImGui::MenuItem("Preferences")) {
						openPopUp<mwetris::ui::scene::Settings>();
					}
				});
				ImGui::Menu("In Game", []() {
					if (ImGui::MenuItem("Pause", "P OR Pause")) {}
					if (ImGui::MenuItem("Restart", "F2")) {}
				});
				ImGui::Menu("Network Game", []() {
					if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
					if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
					ImGui::Separator();
					if (ImGui::MenuItem("Cut", "CTRL+X")) {}
					if (ImGui::MenuItem("Copy", "CTRL+C")) {}
					if (ImGui::MenuItem("Paste", "CTRL+V")) {}
				});
				ImGui::Menu("Help", [&]() {
					if (ImGui::MenuItem("About")) {
						openPopUp<mwetris::ui::scene::About>();
					}
				});
			});
		});

		ImGui::PopFont();
	}

	void TetrisWindow::imGuiEventUpdate(const SDL_Event& windowEvent) {
		if (!play_->eventUpdate(windowEvent)) {
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

}
