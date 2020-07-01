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

#include <string_view>

namespace tetris::ui {

	namespace {

		const ImGuiWindowFlags ImGuiNoWindow = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMove;

		constexpr std::string_view toString(scene::Event event) {
			switch (event) {
				case scene::Event::Menu:
					return "MENU";
				case scene::Event::Play:
					return "PLAY";
				case scene::Event::HighScore:
					return "HIGHSCORE";
				case scene::Event::CustomPlay:
					return "CUSTOM";
				case scene::Event::Settings:
					return "SETTINGS";
					//case scene::Event::Menu:
						//return "NEW_HIGHSCORE";
				case scene::Event::NetworkPlay:
					return "NETWORK";
			}
			return "UNKNOWN";
		}

	}

	void TetrisWindow::changePage(scene::Event event) {
		spdlog::info("[TetrisWindow] open {} page", toString(event));
		dispatcher_->trigger(event);
	}

	TetrisWindow::TetrisWindow()
		: dispatcher_{std::make_shared<entt::dispatcher>()}
		, sceneStateMachine_{dispatcher_} {

		menuHeight_ = tetris::TetrisData::getInstance().getWindowBarHeight();
		manTexture_ = tetris::TetrisData::getInstance().getHumanSprite();
		noManTexture_ = tetris::TetrisData::getInstance().getCrossSprite();
		aiTexture_ = tetris::TetrisData::getInstance().getComputerSprite();
		background_ = tetris::TetrisData::getInstance().getBackgroundSprite();
		labelColor_ = tetris::TetrisData::getInstance().getLabelTextColor();
		buttonTextColor_ = tetris::TetrisData::getInstance().getButtonTextColor();

		nbrHumans_ = 1;
		nbrAis_ = 2;

		setPosition(tetris::TetrisData::getInstance().getWindowPositionX(), tetris::TetrisData::getInstance().getWindowPositionY());
		setSize(tetris::TetrisData::getInstance().getWindowWidth(), tetris::TetrisData::getInstance().getWindowHeight());
		setResizeable(tetris::TetrisData::getInstance().getWindowWidth());
		setTitle("MWetris");
		setIcon(tetris::TetrisData::getInstance().getWindowIcon());
		setBordered(tetris::TetrisData::getInstance().isWindowBordered());
		setShowDemoWindow(true);
		//ImGui::GetStyle().

		//ImGui::PushStyleColor(ImGuiCol_Button, buttonTextColor_.Value);

		activeAis_[0] = findAiDevice(TetrisData::getInstance().getAi1Name());
		activeAis_[1] = findAiDevice(TetrisData::getInstance().getAi2Name());
		activeAis_[2] = findAiDevice(TetrisData::getInstance().getAi3Name());
		activeAis_[3] = findAiDevice(TetrisData::getInstance().getAi4Name());

		devices_.push_back(std::make_shared<game::Keyboard>("Keyboard 1", SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_UP, SDLK_RCTRL));
		devices_.push_back(std::make_shared<game::Keyboard>("Keyboard 2", SDLK_s, SDLK_a, SDLK_d, SDLK_w, SDLK_LCTRL));
		
		sceneStateMachine_.emplace<scene::Menu>();
		sceneStateMachine_.emplace<scene::Play>();
		sceneStateMachine_.emplace<scene::Network>();
		sceneStateMachine_.emplace<scene::Settings>();
		sceneStateMachine_.emplace<scene::HighScore>();
		
		dispatcher_->sink<scene::Event>().connect<&TetrisWindow::handleSceneMenuEvent>(*this);
	}

	TetrisWindow::~TetrisWindow() {
	}
	
	std::vector<game::DevicePtr> TetrisWindow::getCurrentDevices() const {
		std::vector<game::DevicePtr> playerDevices(devices_.begin(), devices_.begin() + nbrHumans_);

		for (int i = 0; i < nbrAis_; ++i) {
			if (activeAis_[i]) {
				playerDevices.push_back(activeAis_[i]);
			}
		}

		return playerDevices;
	}

	game::DevicePtr TetrisWindow::findHumanDevice(std::string name) const {
		for (const auto& device : devices_) {
			if (device->getName() == name) {
				return device;
			}
		}
		return devices_[0];
	}

	game::DevicePtr TetrisWindow::findAiDevice(std::string name) const {
		auto ais = TetrisData::getInstance().getAiVector();
		for (const Ai& ai : ais) {
			if (ai.getName() == name) {
				return std::make_shared<game::Computer>(ai);
			}
		}
		return std::make_shared<game::Computer>(ais.back());
	}

	void TetrisWindow::resumeGame() {
		int rows = TetrisData::getInstance().getActiveLocalGameRows();
		int columns = TetrisData::getInstance().getActiveLocalGameColumns();

		int ais = 0;
		int humans = 0;

		std::vector<game::PlayerData> playerDataVector = TetrisData::getInstance().getActiveLocalGamePlayers();
		for (auto& playerData : playerDataVector) {
			if (playerData.ai_) {
				playerData.device_ = findAiDevice(playerData.deviceName_);
				++ais;
			} else {
				playerData.device_ = findHumanDevice(playerData.deviceName_);
				++humans;
			}
		}
		game_.resumeGame(columns, rows, playerDataVector);
		nbrAis_ = ais;
		nbrHumans_ = humans;
	}

	void TetrisWindow::initPreLoop() {
		sdl::ImGuiWindow::initPreLoop();
		auto& io{ImGui::GetIO()};

		background_.bindTexture();
		tetris::TetrisData::getInstance().bindTextureFromAtlas();
		gameComponent_ = std::make_unique<graphic::GameComponent>(game_);
		//ImGui::GetStyle().WindowBorderSize = 0;

		TetrisData::getInstance().getImGuiButtonFont();
		TetrisData::getInstance().getImGuiDefaultFont();
		TetrisData::getInstance().getImGuiHeaderFont();
	}

	void TetrisWindow::imGuiPreUpdate(const std::chrono::high_resolution_clock::duration& deltaTime) {
		if (true) {
			return;
		}
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				
		graphic.clearDraw();
		auto w = (float) getWidth();
		auto h = (float) getHeight();

		auto deltaTimeSeconds = std::chrono::duration<double>(deltaTime).count();
		if (h > 0 && w > 0) {
			graphic.pushMatrix(glm::ortho(0.f, w, 0.f, h));
			graphic.addRectangleImage({0.f, 0.f}, {w, h}, background_.getTextureView());
			gameComponent_->draw(graphic, getWidth(), getHeight() - menuHeight_, deltaTimeSeconds);
			graphic.draw(getShader());
		}
		
		game_.update(deltaTimeSeconds);
	}
	
	void TetrisWindow::imGuiUpdate(const std::chrono::high_resolution_clock::duration& deltaTime) {
		ImVec4 clear_color{0.45f, 0.55f, 0.6f, 1.f};
		auto context = SDL_GL_GetCurrentContext();
	
		ImGui::PushFont(tetris::TetrisData::getInstance().getImGuiDefaultFont());
		ImGui::PopFont();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, {0, 0, 0, 0});

		ImGui::SetNextWindowPos({0.f,0.f});
		auto [width, height] = sdl::Window::getSize();
		ImGui::SetNextWindowSize({static_cast<float>(width), static_cast<float>(height)});
		
		ImGui::Window("Main", nullptr, ImGuiNoWindow, [&]() {
			ImGui::ImageBackground(background_.getTextureView());
			sceneStateMachine_.imGuiUpdate(deltaTime);
		});
		ImGui::PopStyleColor();
		ImGui::PopStyleVar(3);
	}

	void TetrisWindow::imGuiPostUpdate(const std::chrono::high_resolution_clock::duration& deltaTime) {
		dispatcher_->update();
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
		for (auto& device : devices_) {
			device->eventUpdate(windowEvent);
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
