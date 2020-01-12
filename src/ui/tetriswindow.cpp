#include "tetriswindow.h"

#include "imguiextra.h"
#include "../logger.h"
#include "../tetrisdata.h"

namespace tetris {

	namespace {

		const ImGuiWindowFlags ImGuiNoWindow = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMove;

		bool ComboAi(const char* name, int& item, const std::vector<Ai>& ais, ImGuiComboFlags flags = 0) {
			int oldItem = item;
			if (ImGui::BeginCombo(name, ais[item].getName().c_str(), flags))
			{
				size_t size = ais.size();
				for (int n = 0; n < size; ++n)
				{
					bool is_selected = (item == n);
					if (ImGui::Selectable(ais[n].getName().c_str(), is_selected)) {
						item = n;
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			return oldItem != item;
		}

		constexpr const char* toString(TetrisWindow::Page page) {
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
			return "Unknown";
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
	}

	TetrisWindow::~TetrisWindow() {
	}

	void TetrisWindow::initPreLoop() {
		sdl::ImGuiWindow::initPreLoop();
		auto& io{ImGui::GetIO()};
		
		defaultFont_ = io.Fonts->AddFontFromFileTTF("fonts/Ubuntu-B.ttf", 16);
		headerFont_ = io.Fonts->AddFontFromFileTTF("fonts/Ubuntu-B.ttf", 50);
		buttonFont_ = io.Fonts->AddFontFromFileTTF("fonts/Ubuntu-B.ttf", 35);

		tetris::TetrisData::getInstance().bindTextureFromAtlas();

		//ImGui::GetStyle().WindowBorderSize = 0;
	}

	void TetrisWindow::beginBar() {
		//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2, 2));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, tetris::TetrisData::getInstance().getWindowBarColor().toImU32());

		ImGui::Bar(menuHeight_, tetris::TetrisData::getInstance().getWindowBarColor().toImU32());
	}

	void TetrisWindow::endBar() {
		ImGui::Dummy({0.f, tetris::TetrisData::getInstance().getWindowBarHeight()});

		ImGui::SetCursorPos({0.f, tetris::TetrisData::getInstance().getWindowBarHeight()});

		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
	}
	
	void TetrisWindow::imGuiUpdate(const std::chrono::high_resolution_clock::duration& deltaTime) {
		ImVec4 clear_color{0.45f, 0.55f, 0.6f, 1.f};
		auto context = SDL_GL_GetCurrentContext();
	
		ImGui::PushFont(defaultFont_);
		ImGui::PopFont();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);

		ImGui::SetNextWindowPos({0.f,0.f});
		auto [width, height] = sdl::Window::getSize();
		ImGui::SetNextWindowSize({static_cast<float>(width), static_cast<float>(height)});
		ImGui::Window("Main", nullptr, ImGuiNoWindow, [&]() {
			ImGui::ImageBackground(background_);
			switch (currentPage_) {
				case Page::MENU:
					menuPage();
					break;
				case Page::PLAY:
					playPage();
					break;
				case Page::HIGHSCORE:
					highscorePage();
					break;
				case Page::CUSTOM:
					customGamePage();
					break;
				case Page::SETTINGS:
					settingsPage();
					break;
				case Page::NEW_HIGHSCORE:
					break;
				case Page::NETWORK:
					networkPage();
					break;
			}
		});
		ImGui::PopStyleVar(3);
	}

	void TetrisWindow::eventUpdate(const SDL_Event& windowEvent) {
		sdl::ImGuiWindow::eventUpdate(windowEvent);

		switch (windowEvent.type) {
			case SDL_WINDOWEVENT:
				switch (windowEvent.window.event) {
					case SDL_WINDOWEVENT_RESIZED:
						break;
					case SDL_WINDOWEVENT_LEAVE:
						break;
					case SDL_WINDOWEVENT_CLOSE:
						sdl::Window::quit();
				}
				break;
			case SDL_QUIT:
				sdl::Window::quit();
				break;
		}
	}

	void TetrisWindow::menuPage() {
		beginBar();
		endBar();

		ImGui::PushFont(headerFont_);
		ImGui::TextColored(labelColor_, "MWetris");
		ImGui::PopFont();

		pushButtonStyle();
		
		//ImGui::Indent(10.f);
		ImVec2 dummy{0.0f, 5.0f};
	
		if (ImGui::Button("Play")) {
			changePage(Page::PLAY);
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
		beginBar();
		pushButtonStyle();
		ImGui::PushFont(buttonFont_);
	
		if (ImGui::Button("Menu", {100.5f, menuHeight_})) {
			changePage(Page::MENU);
		}
		ImGui::SameLine();
		ImGui::Button("Restart", {120.5f, menuHeight_});
		ImGui::SameLine();
		ImGui::ManButton("Human2", nbrHumans_, 4, noManTexture_, manTexture_, {menuHeight_, menuHeight_});
		ImGui::SameLine();
		ImGui::ManButton("Ai2", nbrAis_, 4, noManTexture_, aiTexture_, {menuHeight_, menuHeight_});
		ImGui::PopFont();

		popButtonStyle();

		endBar();
		ImGui::Dummy({0.0f, tetris::TetrisData::getInstance().getWindowBarHeight()});
		
		imGuiCanvas({300, 300}, [&]() {
			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			
			graphic.clearDraw();
			graphic.addCircle({0, 0}, 1, sdl::RED);
			graphic.addRectangle({-1.f, 0.9f}, {2.f, 0.1f}, sdl::BLUE);
			graphic.draw(getShader());
		});
	}

	void TetrisWindow::highscorePage() {
		beginBar();
		pushButtonStyle();
		if (ImGui::Button("Menu", {100.5f, menuHeight_})) {
			changePage(Page::MENU);
		}
		popButtonStyle();
		endBar();

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
		beginBar();
		pushButtonStyle();
		if (ImGui::Button("Menu", {100.5f, menuHeight_})) {
			changePage(Page::MENU);
		}
		popButtonStyle();
		endBar();
	
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
		beginBar();
		ImGui::PushFont(buttonFont_);
		if (ImGui::Button("Menu", {100.5f, menuHeight_})) {
			changePage(Page::MENU);
		}
		ImGui::PopFont();
		endBar();
	}

	void TetrisWindow::networkPage() {
		beginBar();
		pushButtonStyle();
		if (ImGui::Button("Menu", {100.5f, menuHeight_})) {
			changePage(Page::MENU);
		}
		popButtonStyle();
		endBar();

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
