#include "imguiwindow.h"

#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include "imguiextra.h"
#include "../logger.h"
#include "../tetrisdata.h"

namespace tetris {

	namespace {

		const ImGuiWindowFlags ImGuiNoWindow = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMove;

		const ImGuiWindowFlags ImGuiNoWindow2 = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

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

		constexpr const char* toString(ImGuiWindow::Page page) {
			switch (page) {
			case ImGuiWindow::Page::MENU:
				return "MENU";
			case ImGuiWindow::Page::PLAY:
				return "PLAY";
			case ImGuiWindow::Page::HIGHSCORE:
				return "HIGHSCORE";
			case ImGuiWindow::Page::CUSTOM:
				return "CUSTOM";
			case ImGuiWindow::Page::SETTINGS:
				return "SETTINGS";
			case ImGuiWindow::Page::NEW_HIGHSCORE:
				return "NEW_HIGHSCORE";
			case ImGuiWindow::Page::NETWORK:
				return "NETWORK";
			}
			return "Unknown";
		}

	}

	void ImGuiWindow::changePage(Page page) {
		logger()->info("[ImGuiWindow] open {} page", toString(page));
		currentPage_ = page;
	}

	void ImGuiWindow::pushButtonStyle() {
		ImGui::PushFont(buttonFont_);
		ImGui::PushStyleColor(ImGuiCol_Text, tetris::TetrisData::getInstance().getButtonTextColor().Value);
		ImGui::PushStyleColor(ImGuiCol_Border, tetris::TetrisData::getInstance().getButtonBorderColor().Value);
		ImGui::PushStyleColor(ImGuiCol_Button, tetris::TetrisData::getInstance().getButtonBackgroundColor().Value);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, tetris::TetrisData::getInstance().getButtonHoverColor().Value);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, tetris::TetrisData::getInstance().getButtonFocusColor().Value);
	}

	void ImGuiWindow::popButtonStyle() {
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		ImGui::PopFont();
	}

	ImGuiWindow::ImGuiWindow() {
		show_demo_window = true;
		show_another_window = false;
		initiatedOpenGl_ = false;
		initiatedSdl_ = false;
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

		//ImGui::GetStyle().

		//ImGui::PushStyleColor(ImGuiCol_Button, buttonTextColor_.Value);

	}

	ImGuiWindow::~ImGuiWindow() {
		if (initiatedOpenGl_) {
			ImGui_ImplOpenGL3_Shutdown();
		}
		if (initiatedSdl_) {
			ImGui_ImplSDL2_Shutdown();
			ImGui::DestroyContext();
		}
	}

	void ImGuiWindow::initOpenGl() {
		Window::setOpenGlVersion(3, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		Window::initOpenGl();
	}

	void ImGuiWindow::initPreLoop() {
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();

		ImGui_ImplSDL2_Init(sdl::Window::getSdlWindow());
		initiatedSdl_ = true;
		const char* glsl_version = "#version 130";
		ImGui_ImplOpenGL3_Init(glsl_version);
		initiatedOpenGl_ = true;

		//defaultFont_ = io.Fonts->AddFontFromFileTTF("fonts/Ubuntu-B.ttf", 16);
		headerFont_ = io.Fonts->AddFontFromFileTTF("fonts/Ubuntu-B.ttf", 50);
		buttonFont_ = io.Fonts->AddFontFromFileTTF("fonts/Ubuntu-B.ttf", 35);
		defaultFont_ = io.Fonts->AddFontDefault();

		tetris::TetrisData::getInstance().bindTextureFromAtlas();

		//ImGui::GetStyle().WindowBorderSize = 0;
	}

	void ImGuiWindow::beginBar() {
		//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2, 2));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, tetris::TetrisData::getInstance().getWindowBarColor().Value);

		ImGui::Bar(menuHeight_, tetris::TetrisData::getInstance().getWindowBarColor().Value);
	}

	void ImGuiWindow::endBar() {
		ImGui::Dummy({ 0.0f, tetris::TetrisData::getInstance().getWindowBarHeight() });

		ImGui::SetCursorPos({ 0.0f, tetris::TetrisData::getInstance().getWindowBarHeight() });

		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
	}

	void ImGuiWindow::beginMain() {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
	
		ImGui::SetNextWindowPos({ 0.f, 0.f });
		auto [width, height] = sdl::Window::getSize();
		ImGui::SetNextWindowSize({ (float)width, (float)height });

		ImGui::Begin("Main", nullptr, ImGuiNoWindow);
	}

	void ImGuiWindow::endMain() {
		ImGui::End();

		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
	}

	void ImGuiWindow::update(double deltaTime) {
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
		auto context = SDL_GL_GetCurrentContext();

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(sdl::Window::getSdlWindow());
		ImGui::NewFrame();
	
		ImGui::PushFont(defaultFont_);
		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);
		ImGui::PopFont();

		const ImGuiWindowFlags ImGuiNoWindow = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMove;
		const ImGuiWindowFlags ImGuiInnerWindow = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

		beginMain();
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
		endMain();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void ImGuiWindow::eventUpdate(const SDL_Event& windowEvent) {
		ImGui_ImplSDL2_ProcessEvent(windowEvent);

		switch (windowEvent.type) {
			case SDL_WINDOWEVENT:
				switch (windowEvent.window.event) {
					case SDL_WINDOWEVENT_RESIZED:
						resize(windowEvent.window.data1, windowEvent.window.data2);
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

	void ImGuiWindow::resize(int width, int height) {
		glViewport(0, 0, width, height);
	}


	void ImGuiWindow::menuPage() {
		beginBar();
		endBar();

		ImGui::PushFont(headerFont_);
		ImGui::TextColored(labelColor_, "MWetris");
		ImGui::PopFont();

		pushButtonStyle();
		
		//ImGui::Indent(10.f);
		ImVec2 dummy = { 0.0f, 5.0f };
	
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

	void ImGuiWindow::playPage() {
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
	}

	void ImGuiWindow::highscorePage() {
		beginBar();
		pushButtonStyle();
		if (ImGui::Button("Menu", { 100.5f, menuHeight_ })) {
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

		auto& highscores = TetrisData::getInstance().getHighscoreRecordVector();

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

	void ImGuiWindow::settingsPage() {
		beginBar();
		pushButtonStyle();
		if (ImGui::Button("Menu", { 100.5f, menuHeight_ })) {
			changePage(Page::MENU);
		}
		popButtonStyle();
		endBar();
	
		ImGui::PushFont(headerFont_);
		ImGui::TextColored(labelColor_, "Settings");
		ImGui::PopFont();

		ImGui::Indent(10.f);
		ImGui::Dummy({ 0.0f, 5.0f });

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


	void ImGuiWindow::customGamePage() {
		beginBar();
		ImGui::PushFont(buttonFont_);
		if (ImGui::Button("Menu", { 100.5f, menuHeight_ })) {
			changePage(Page::MENU);
		}
		ImGui::PopFont();
		endBar();
	}

	void ImGuiWindow::networkPage() {
		beginBar();
		pushButtonStyle();
		if (ImGui::Button("Menu", { 100.5f, menuHeight_ })) {
			changePage(Page::MENU);
		}
		popButtonStyle();
		endBar();

		ImGui::Indent(10.f);
		ImGui::Dummy({ 0.0f, 5.0f });

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
		ImGui::Dummy({ 0.0f, 5.0f });

		if (ImGui::Button("Connect", { 100.5f, menuHeight_ })) {

		}
		ImGui::LoadingBar();

		ImGui::PopFont();
	}

} // Namespace tetris.