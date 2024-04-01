#include "mainwindow.h"

#include <configuration.h>

#include <ui/networkdebugwindow.h>
#include <ui/tetriswindow.h>
#include <network/debugclient.h>

#include <ui/tetriswindow.h>

MainWindow::MainWindow(const Config& config) 
	: config_{config} {

	setPosition(mwetris::Configuration::getInstance().getWindowPositionX(), mwetris::Configuration::getInstance().getWindowPositionY());
	setSize(mwetris::Configuration::getInstance().getWindowWidth(), mwetris::Configuration::getInstance().getWindowHeight());
	setResizeable(true);
	setTitle("MWetris");
	setIcon(mwetris::Configuration::getInstance().getWindowIcon());
	setBordered(mwetris::Configuration::getInstance().isWindowBordered());
	setShowDemoWindow(config.showDemoWindow);
}

MainWindow::~MainWindow() {
	mwetris::Configuration::getInstance().quit();
}

void MainWindow::initPreLoop() {
	sdl::ImGuiWindow::initPreLoop();

	ImGui::GetIO().Fonts->AddFontFromFileTTF("fonts/Ubuntu-B.ttf", 12); // Used by demo window.
	
	mwetris::Configuration::getInstance().getImGuiButtonFont();
	mwetris::Configuration::getInstance().getImGuiDefaultFont();
	mwetris::Configuration::getInstance().getImGuiHeaderFont();

	deviceManager_ = std::make_shared<mwetris::game::DeviceManager>();
	debugServer_ = std::make_shared<mwetris::network::DebugServer>();

	for (int i = 0; i < config_.windows; ++i) {
		auto debugClient = debugServer_->createClient();
		auto type = (i == 0) ? mwetris::ui::TetrisWindow::Type::MainWindow : mwetris::ui::TetrisWindow::Type::SecondaryWindow;
		std::string name = (i == 0) ? "MainWindow" : fmt::format("SecondaryWindow{}", i);
		subWindows_.push_back(std::make_unique<mwetris::ui::TetrisWindow>(name, type , *this,
			deviceManager_,
			debugClient
		));
	}

	if (config_.showDebugWindow) {
		subWindows_.push_back(std::make_unique<mwetris::ui::NetworkDebugWindow>(debugServer_));
	}
}

void MainWindow::eventUpdate(const SDL_Event& windowEvent) {
	sdl::ImGuiWindow::eventUpdate(windowEvent);

	deviceManager_->eventUpdate(windowEvent);
}

void MainWindow::imGuiUpdate(const sdl::DeltaTime& deltaTime) {
	for (auto& subWindow : subWindows_) {
		subWindow->imGuiUpdate(deltaTime);
	}
	debugServer_->update(deltaTime);
}

void MainWindow::imGuiEventUpdate(const SDL_Event& windowEvent) {
	switch (windowEvent.type) {
		case SDL_WINDOWEVENT:
			switch (windowEvent.window.event) {
				case SDL_WINDOWEVENT_CLOSE:
					quit();
					break;
			}
			break;
		case SDL_KEYDOWN:
			switch (windowEvent.key.keysym.sym) {
				case SDLK_ESCAPE:
					quit();
					break;
			}
			break;
		case SDL_QUIT:
			quit();
			break;
	}

	for (auto& subWindow : subWindows_) {
		subWindow->imGuiEventUpdate(windowEvent);
	}
}
