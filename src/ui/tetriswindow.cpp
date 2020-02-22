#include "tetriswindow.h"

#include "imguiextra.h"
#include "../logger.h"
#include "../tetrisdata.h"
#include "../graphic/gamecomponent.h"
#include "../game/computer.h"
#include "../game/keyboard.h"

#include <sdl/imguiauxiliary.h>

#include <string_view>

namespace tetris {

	namespace {

		const ImGuiWindowFlags ImGuiNoWindow = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMove;

		bool ComboAi(const char* name, int& item, const std::vector<Ai>& ais, ImGuiComboFlags flags = 0) {
			int oldItem = item;
			ImGui::ComboScoped(name, ais[item].getName().c_str(), flags, [&]() {
				auto size = ais.size();
				for (int n = 0; n < size; ++n)
				{
					bool isSelected = (item == n);
					if (ImGui::Selectable(ais[n].getName().c_str(), isSelected)) {
						item = n;
					}
					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}
			});
			return oldItem != item;
		}

		constexpr std::string_view toString(TetrisWindow::Page page) {
			switch (page) {
			case TetrisWindow::Page::MENU:
				return "MENU";
			case TetrisWindow::Page::PLAY:
				return "PLAY";
			case TetrisWindow::Page::HIGHSCORE:
				return "HIGHSCORE";
			case TetrisWindow::Page::CUSTOM:
				return "CUSTOM";
			case TetrisWindow::Page::SETTINGS:
				return "SETTINGS";
			case TetrisWindow::Page::NEW_HIGHSCORE:
				return "NEW_HIGHSCORE";
			case TetrisWindow::Page::NETWORK:
				return "NETWORK";
			}
			return "UNKNOWN";
		}

	}

	void TetrisWindow::changePage(Page page) {
		logger()->info("[TetrisWindow] open {} page", toString(page));
		currentPage_ = page;
	}

	void TetrisWindow::pushButtonStyle() {
		ImGui::PushFont(buttonFont_);
		ImGui::PushStyleColor(ImGuiCol_Text, tetris::TetrisData::getInstance().getButtonTextColor().toImU32());
		ImGui::PushStyleColor(ImGuiCol_Border, tetris::TetrisData::getInstance().getButtonBorderColor().toImU32());
		ImGui::PushStyleColor(ImGuiCol_Button, tetris::TetrisData::getInstance().getButtonBackgroundColor().toImU32());
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, tetris::TetrisData::getInstance().getButtonHoverColor().toImU32());
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, tetris::TetrisData::getInstance().getButtonFocusColor().toImU32());
	}

	void TetrisWindow::popButtonStyle() {
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		ImGui::PopFont();
	}

	TetrisWindow::TetrisWindow() {
		menuHeight_ = tetris::TetrisData::getInstance().getWindowBarHeight();
		manTexture_ = tetris::TetrisData::getInstance().getHumanSprite();
		noManTexture_ = tetris::TetrisData::getInstance().getCrossSprite();
		aiTexture_ = tetris::TetrisData::getInstance().getComputerSprite();
		background_ = tetris::TetrisData::getInstance().getBackgroundSprite();
		labelColor_ = tetris::TetrisData::getInstance().getLabelTextColor();
		buttonTextColor_ = tetris::TetrisData::getInstance().getButtonTextColor();

		nbrHumans_ = 1;
		nbrAis_ = 2;
		changePage(Page::MENU);

		setPosition(tetris::TetrisData::getInstance().getWindowPositionX(), tetris::TetrisData::getInstance().getWindowPositionY());
		setWindowSize(tetris::TetrisData::getInstance().getWindowWidth(), tetris::TetrisData::getInstance().getWindowHeight());
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

		devices_.push_back(std::make_shared<Keyboard>("Keyboard 1", SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_UP, SDLK_RCTRL));
		devices_.push_back(std::make_shared<Keyboard>("Keyboard 2", SDLK_s, SDLK_a, SDLK_d, SDLK_w, SDLK_LCTRL));
	}

	TetrisWindow::~TetrisWindow() {
	}

	DevicePtr TetrisWindow::findHumanDevice(std::string name) const {
		for (const auto& device : devices_) {
			if (device->getName() == name) {
				return device;
			}
		}
		return devices_[0];
	}

	DevicePtr TetrisWindow::findAiDevice(std::string name) const {
		auto ais = TetrisData::getInstance().getAiVector();
		for (const Ai& ai : ais) {
			if (ai.getName() == name) {
				return std::make_shared<Computer>(ai);
			}
		}
		return std::make_shared<Computer>(ais.back());
	}

	void TetrisWindow::resumeGame() {
		int rows = TetrisData::getInstance().getActiveLocalGameRows();
		int columns = TetrisData::getInstance().getActiveLocalGameColumns();

		int ais = 0;
		int humans = 0;

		std::vector<PlayerData> playerDataVector = TetrisData::getInstance().getActiveLocalGamePlayers();
		for (PlayerData& playerData : playerDataVector) {
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
		
		defaultFont_ = io.Fonts->AddFontFromFileTTF("fonts/Ubuntu-B.ttf", 16);
		headerFont_ = io.Fonts->AddFontFromFileTTF("fonts/Ubuntu-B.ttf", 50);
		buttonFont_ = io.Fonts->AddFontFromFileTTF("fonts/Ubuntu-B.ttf", 35);
		background_.bindTexture();
		tetris::TetrisData::getInstance().bindTextureFromAtlas();
		drawBoard_.setFont(headerFont_);
		//gameComponent_ = std::make_unique<GameComponent>(game_);
		//ImGui::GetStyle().WindowBorderSize = 0;
	}

	void TetrisWindow::imGuiPreUpdate(const std::chrono::high_resolution_clock::duration& deltaTime) {
		const auto& shader = getShader();
		getShader().useProgram();
		
		if (currentPage_ != Page::PLAY) {
			return;
		}
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				
		graphic.clearDraw();
		graphic.setMatrix(glm::ortho(0.f, (float) getWidth(), 0.f, (float) getHeight()));
		//graphic.setMatrix(glm::ortho(0.f, (float) 800, 0.f, (float) 800));
		//graphic.setMatrix(glm::ortho(-2.0, 2.0, -2.0, 2.0));
		graphic.addRectangleImage({0.f, 0.f}, {getWidth(), getHeight()}, background_.getTextureView());
		graphic.addCircle({0, 0}, 1, sdl::RED);
		graphic.addRectangle({-1.f, 0.9f}, {2.f, 0.1f}, sdl::BLUE);

		graphic.addRectangle({400, 400}, {100, 100}, sdl::BLUE);
		graphic.draw(getShader());
		drawBoard_.draw(graphic);
		graphic.draw(shader);
		
		//gameComponent_->draw(getWidth(), getHeight(), std::chrono::duration<double>(deltaTime).count());
	}
	
	void TetrisWindow::imGuiUpdate(const std::chrono::high_resolution_clock::duration& deltaTime) {
		ImVec4 clear_color{0.45f, 0.55f, 0.6f, 1.f};
		auto context = SDL_GL_GetCurrentContext();
	
		ImGui::PushFont(defaultFont_);
		ImGui::PopFont();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, {0, 0, 0, 0});

		ImGui::SetNextWindowPos({0.f,0.f});
		auto [width, height] = sdl::Window::getSize();
		ImGui::SetNextWindowSize({static_cast<float>(width), static_cast<float>(height)});
		ImGui::Window("Main", nullptr, ImGuiNoWindow, [&]() {
			switch (currentPage_) {
				case Page::MENU:
					ImGui::ImageBackground(background_.getTextureView());
					menuPage();
					break;
				case Page::PLAY:
					playPage();
					break;
				case Page::HIGHSCORE:
					ImGui::ImageBackground(background_.getTextureView());
					highscorePage();
					break;
				case Page::CUSTOM:
					ImGui::ImageBackground(background_.getTextureView());
					customGamePage();
					break;
				case Page::SETTINGS:
					ImGui::ImageBackground(background_.getTextureView());
					settingsPage();
					break;
				case Page::NEW_HIGHSCORE:
					ImGui::ImageBackground(background_.getTextureView());
					break;
				case Page::NETWORK:
					ImGui::ImageBackground(background_.getTextureView());
					networkPage();
					break;
			}
		});
		ImGui::PopStyleColor();
		ImGui::PopStyleVar(3);
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
					//hexCanvas_.eventUpdate(windowEvent);
				}
				break;
			case SDL_KEYUP:
				if (!io.WantCaptureKeyboard) {
					//hexCanvas_.eventUpdate(windowEvent);
				}
				break;
			case SDL_KEYDOWN:
				if (!io.WantCaptureKeyboard) {
					//hexCanvas_.eventUpdate(windowEvent);
				}
				break;
			case SDL_QUIT:
				sdl::Window::quit();
				break;
		}
	}

	void TetrisWindow::menuPage() {
		ImGui::Bar(menuHeight_, []() {});

		ImGui::PushFont(headerFont_);
		ImGui::TextColored(labelColor_, "MWetris");
		ImGui::PopFont();

		pushButtonStyle();

		//ImGui::Indent(10.f);
		ImVec2 dummy{0.0f, 5.0f};

		if (ImGui::Button("Play")) {
			changePage(Page::PLAY);
			resumeGame();
		}
		ImGui::Dummy(dummy);
		if (ImGui::Button("Custom Play")) {
			changePage(Page::CUSTOM);
		}
		ImGui::Dummy(dummy);
		if (ImGui::Button("Network Play")) {
			changePage(Page::NETWORK);
		}
		ImGui::Dummy(dummy);
		if (ImGui::Button("Highscore")) {
			changePage(Page::HIGHSCORE);
		}
		ImGui::Dummy(dummy);
		if (ImGui::Button("Settings")) {
			changePage(Page::SETTINGS);
		}
		ImGui::Dummy(dummy);
		if (ImGui::Button("Exit")) {
			logger()->info("[ImGuiWindow] pushed exit button");
			sdl::Window::quit();
		}

		popButtonStyle();
	}

	void TetrisWindow::playPage() {
		ImGui::Bar(menuHeight_, [&]() {
			pushButtonStyle();
			ImGui::PushFont(buttonFont_);
	
			if (ImGui::Button("Menu", {100.5f, menuHeight_})) {
				changePage(Page::MENU);
			}
			ImGui::SameLine();
			ImGui::Button("Restart", {120.5f, menuHeight_});
			ImGui::SameLine();
			ImGui::ManButton("Human2", nbrHumans_, 4, noManTexture_.getTextureView(), manTexture_.getTextureView(), {menuHeight_, menuHeight_});
			ImGui::SameLine();
			ImGui::ManButton("Ai2", nbrAis_, 4, noManTexture_.getTextureView(), aiTexture_.getTextureView(), {menuHeight_, menuHeight_});
			ImGui::PopFont();

			popButtonStyle();
		});
		drawBoard_.imGui((float) getWidth());
	}

	void TetrisWindow::highscorePage() {
		ImGui::Bar(menuHeight_, [&]() {
			pushButtonStyle();
			if (ImGui::Button("Menu", {100.5f, menuHeight_})) {
				changePage(Page::MENU);
			}
			popButtonStyle();
		});

		ImGui::PushFont(headerFont_);
		ImGui::TextColored(labelColor_, "Highscore");
		ImGui::PopFont();

		ImGui::PushFont(defaultFont_);
		ImGui::Columns(6, "Highscore");
		ImGui::Separator();
		ImGui::Text("Ranking"); ImGui::NextColumn();
		ImGui::Text("Points"); ImGui::NextColumn();
		ImGui::Text("Name"); ImGui::NextColumn();
		ImGui::Text("Rows"); ImGui::NextColumn();
		ImGui::Text("Level"); ImGui::NextColumn();
		ImGui::Text("Date"); ImGui::NextColumn();
		ImGui::Separator();

		auto highscores = TetrisData::getInstance().getHighscoreRecordVector();

		int rankNbr = 1;
		for (const auto& highscore : highscores) {
			ImGui::Text(std::to_string(rankNbr++).c_str()); ImGui::NextColumn();
			ImGui::Text(std::to_string(highscore.points_).c_str()); ImGui::NextColumn();
			ImGui::Text(highscore.name_.c_str()); ImGui::NextColumn();
			ImGui::Text(std::to_string(highscore.rows_).c_str()); ImGui::NextColumn();
			ImGui::Text(std::to_string(highscore.level_).c_str()); ImGui::NextColumn();
			ImGui::Text(highscore.date_.c_str()); ImGui::NextColumn();
		}
		ImGui::Separator();
		ImGui::PopFont();
	}	

	void TetrisWindow::settingsPage() {
		ImGui::Bar(menuHeight_, [&]() {
			pushButtonStyle();
			if (ImGui::Button("Menu", {100.5f, menuHeight_})) {
				changePage(Page::MENU);
			}
			popButtonStyle();
		});
	
		ImGui::PushFont(headerFont_);
		ImGui::TextColored(labelColor_, "Settings");
		ImGui::PopFont();

		ImGui::Indent(10.f);
		ImGui::Dummy({0.0f, 5.0f});

		ImGui::PushFont(defaultFont_);	
		bool check = tetris::TetrisData::getInstance().isWindowBordered();
		if (ImGui::Checkbox("Border around window", &check)) {
			tetris::TetrisData::getInstance().setWindowBordered(check);
		}

		check = tetris::TetrisData::getInstance().isFullscreenOnDoubleClick();
		if (ImGui::Checkbox("Fullscreen on double click", &check)) {
			tetris::TetrisData::getInstance().setFullscreenOnDoubleClick(check);
		}

		check = tetris::TetrisData::getInstance().isMoveWindowByHoldingDownMouse();
		if (ImGui::Checkbox("Move the window by holding down left mouse button", &check)) {
			tetris::TetrisData::getInstance().setMoveWindowByHoldingDownMouse(check);
		}
	
		check = tetris::TetrisData::getInstance().isWindowVsync();
		if (ImGui::Checkbox("Vsync", &check)) {
			tetris::TetrisData::getInstance().setWindowVsync(check);
		}

		check = tetris::TetrisData::getInstance().isLimitFps();
		if (ImGui::Checkbox("FpsLimiter", &check)) {
			tetris::TetrisData::getInstance().setLimitFps(check);
		}

		check = tetris::TetrisData::getInstance().isWindowPauseOnLostFocus();
		if (ImGui::Checkbox("Paus on lost focus", &check)) {
			tetris::TetrisData::getInstance().setWindowPauseOnLostFocus(check);
		}

		check = tetris::TetrisData::getInstance().isShowDownBlock();
		if (ImGui::Checkbox("Show down block", &check)) {
			tetris::TetrisData::getInstance().setShowDownBlock(check);
		}

		ImGui::PushFont(headerFont_);
		ImGui::TextColored(labelColor_, "Ai players");
		ImGui::PopFont();

		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.20f);

		const auto& ais = TetrisData::getInstance().getAiVector();
		int nbrAi1 = std::find_if(ais.begin(), ais.end(), [](const Ai& ai) { return ai.getName() == TetrisData::getInstance().getAi1Name(); }) - ais.begin();
		if (ComboAi("Ai1", nbrAi1, ais)) {
			TetrisData::getInstance().setAi1Name(ais[nbrAi1].getName());
		}
		int nbrAi2 = std::find_if(ais.begin(), ais.end(), [](const Ai & ai) { return ai.getName() == TetrisData::getInstance().getAi2Name(); }) - ais.begin();
		if (ComboAi("Ai2", nbrAi2, ais)) {
			TetrisData::getInstance().setAi2Name(ais[nbrAi2].getName());
		}
		int nbrAi3 = std::find_if(ais.begin(), ais.end(), [](const Ai & ai) { return ai.getName() == TetrisData::getInstance().getAi3Name(); }) - ais.begin();
		if (ComboAi("Ai3", nbrAi3, ais)) {
			TetrisData::getInstance().setAi4Name(ais[nbrAi3].getName());
		}
		int nbrAi4 = std::find_if(ais.begin(), ais.end(), [](const Ai & ai) { return ai.getName() == TetrisData::getInstance().getAi4Name(); }) - ais.begin();
		if (ComboAi("Ai4", nbrAi4, ais)) {
			TetrisData::getInstance().setAi4Name(ais[nbrAi4].getName());
		}

		ImGui::PopItemWidth();
		ImGui::PopFont();
	}


	void TetrisWindow::customGamePage() {
		ImGui::Bar(menuHeight_, [&]() {
			ImGui::PushFont(buttonFont_);
			if (ImGui::Button("Menu", {100.5f, menuHeight_})) {
				changePage(Page::MENU);
			}
			ImGui::PopFont();
		});
	}

	void TetrisWindow::networkPage() {
		ImGui::Bar(menuHeight_, [&]() {
			pushButtonStyle();
			if (ImGui::Button("Menu", {100.5f, menuHeight_})) {
				changePage(Page::MENU);
			}
			popButtonStyle();
		});

		ImGui::Indent(10.f);
		ImGui::Dummy({0.0f, 5.0f});

		ImGui::PushFont(defaultFont_);
		static int radioNbr = 0;
		ImGui::RadioButton("Server", &radioNbr, 0);
		ImGui::RadioButton("Client", &radioNbr, 1);	

		ImGui::PushItemWidth(150);
		if (radioNbr == 0) {
			static char ipField[30] = "";
			ImGui::InputText("The ip-number for the server", ipField, 30);
			if (ImGui::BeginPopupContextItem("item context menu", 1)) {
				if (ImGui::Selectable("Copy")) {
					ImGui::SetClipboardText(ipField);
				}
				if (ImGui::Selectable("Paste")) {
					std::strcpy(ipField, ImGui::GetClipboardText());
				}
				ImGui::EndPopup();
			}
		}
		static int port = 5012;
		ImGui::InputInt("The port-number for the server", &port);

		ImGui::PopItemWidth();
		ImGui::Dummy({0.f, 5.f});

		if (ImGui::Button("Connect", {100.5f, menuHeight_})) {

		}
		ImGui::LoadingBar();

		ImGui::PopFont();
	}

} // Namespace tetris.
