#include "mainwindow.h"

#include <configuration.h>

#include <ui/tetriswindow.h>
#include <network/debugclient.h>

MainWindow::MainWindow() {
	setPosition(mwetris::Configuration::getInstance().getWindowPositionX(), mwetris::Configuration::getInstance().getWindowPositionY());
	setSize(mwetris::Configuration::getInstance().getWindowWidth(), mwetris::Configuration::getInstance().getWindowHeight());
	setResizeable(true);
	setTitle("MWetris");
	setIcon(mwetris::Configuration::getInstance().getWindowIcon());
	setBordered(mwetris::Configuration::getInstance().isWindowBordered());
	setShowDemoWindow(true);
}

MainWindow::~MainWindow() {
	mwetris::Configuration::getInstance().quit();
}

void MainWindow::initPreLoop() {
	sdl::ImGuiWindow::initPreLoop();

	auto& io{ImGui::GetIO()};
	io.Fonts->AddFontFromFileTTF("fonts/Ubuntu-B.ttf", 12); // Used by demo window.
	
	mwetris::Configuration::getInstance().getImGuiButtonFont();
	mwetris::Configuration::getInstance().getImGuiDefaultFont();
	mwetris::Configuration::getInstance().getImGuiHeaderFont();

	auto deviceManager = std::make_shared<mwetris::game::DeviceManager>();
	auto debugClient = std::make_shared<mwetris::network::DebugClient>();

	tetrisWindow_ = std::make_unique<mwetris::ui::TetrisWindow>(*this,
		deviceManager,
		debugClient
	);
	tetrisWindow_->initPreLoop();

	subWindows_.push_back(std::make_unique<mwetris::ui::NetworkDebugWindow>(debugClient));
}

void MainWindow::eventUpdate(const SDL_Event& windowEvent) {
	sdl::ImGuiWindow::eventUpdate(windowEvent);

	tetrisWindow_->eventUpdate(windowEvent);
}

void MainWindow::imGuiUpdate(const sdl::DeltaTime& deltaTime) {
	tetrisWindow_->imGuiUpdate(deltaTime);

	for (auto& subWindow : subWindows_) {
		subWindow->imGuiUpdate(deltaTime);
	}
}

void MainWindow::imGuiEventUpdate(const SDL_Event& windowEvent) {
	tetrisWindow_->imGuiEventUpdate(windowEvent);
}
