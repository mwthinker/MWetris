#include "mainwindow.h"

#include <app/configuration.h>

#include <app/ui/networkdebugwindow.h>
#include <app/ui/tetriswindow.h>
#include <app/cnetwork/network.h>

#include <network/tcpclient.h>
#include <network/tcpserver.h>

#include <app/ui/tetriswindow.h>

MainWindow::MainWindow(const Config& config) 
	: config_{config} {

	setPosition(app::Configuration::getInstance().getWindowPositionX(), app::Configuration::getInstance().getWindowPositionY());
	setSize(app::Configuration::getInstance().getWindowWidth(), app::Configuration::getInstance().getWindowHeight());
	setResizeable(true);
	setTitle("MWetris");
	setIcon(app::Configuration::getInstance().getWindowIcon());
	setBordered(app::Configuration::getInstance().isWindowBordered());
	setShowDemoWindow(config.showDemoWindow);
}

MainWindow::~MainWindow() {
	if (server_) {
		server_->stop();
	}
	ioContext_.stop();
	ioContext_.run();
	app::Configuration::getInstance().quit();
}

void MainWindow::initPreLoop() {
	sdl::ImGuiWindow::initPreLoop();

	ImGui::GetIO().Fonts->AddFontFromFileTTF("fonts/Ubuntu-B.ttf", 12); // Used by demo window.
	
	app::Configuration::getInstance().getImGuiButtonFont();
	app::Configuration::getInstance().getImGuiDefaultFont();
	app::Configuration::getInstance().getImGuiHeaderFont();

	deviceManager_ = std::make_shared<app::game::DeviceManager>();

	switch (config_.network) {
		case Network::SingleTcpClient:
			spdlog::info("SingleTcpClient");
			initSingleTcpClient();
			break;
		case Network::TcpServer:
			spdlog::info("TcpServer");
			initTcpServer();
			break;
		case Network::DebugServer:
			spdlog::info("DebugServer");
			initDebugServer();
			break;
	}

	if (config_.showDebugWindow && server_) {
		subWindows_.push_back(std::make_unique<app::ui::NetworkDebugWindow>(server_));
	}
}

void MainWindow::initSingleTcpClient() {
	auto [ip, port] = app::Configuration::getInstance().getNetwork().server;
	for (int i = 0; i < config_.windows; ++i) {
		auto tcpClient = network::TcpClient::connectToServer(ioContext_, ip, port);
		auto type = (i == 0) ? app::ui::TetrisWindow::Type::MainWindow : app::ui::TetrisWindow::Type::SecondaryWindow;
		std::string name = (i == 0) ? "MainWindow" : fmt::format("SecondaryWindow{}", i);
		subWindows_.push_back(std::make_unique<app::ui::TetrisWindow>(name, type, *this,
			deviceManager_,
			std::make_shared<app::cnetwork::Network>(tcpClient)
		));
	}
}

void MainWindow::initTcpServer() {
	auto [ip, port] = app::Configuration::getInstance().getNetwork().server;
	auto settings = network::TcpServer::Settings{
		.port = port
	};
	auto server = std::make_shared<network::TcpServer>(ioContext_, settings);
	for (int i = 0; i < config_.windows; ++i) {
		auto tcpClient = network::TcpClient::connectToServer(ioContext_, ip, port);
		auto type = (i == 0) ? app::ui::TetrisWindow::Type::MainWindow : app::ui::TetrisWindow::Type::SecondaryWindow;
		std::string name = (i == 0) ? "MainWindow" : fmt::format("SecondaryWindow{}", i);
		subWindows_.push_back(std::make_unique<app::ui::TetrisWindow>(name, type, *this,
			deviceManager_,
			std::make_shared<app::cnetwork::Network>(tcpClient)
		));
	}
	server_ = server;
	server_->start();
}

void MainWindow::initDebugServer() {
	auto server = std::make_shared<network::DebugServer>(ioContext_);
	for (int i = 0; i < config_.windows; ++i) {
		auto debugClient = server->addClient();
		auto type = (i == 0) ? app::ui::TetrisWindow::Type::MainWindow : app::ui::TetrisWindow::Type::SecondaryWindow;
		std::string name = (i == 0) ? "MainWindow" : fmt::format("SecondaryWindow{}", i);
		subWindows_.push_back(std::make_unique<app::ui::TetrisWindow>(name, type, *this,
			deviceManager_,
			std::make_shared<app::cnetwork::Network>(debugClient)
		));
	}
	server_ = server;
	server_->start();
}

void MainWindow::eventUpdate(const SDL_Event& windowEvent) {
	sdl::ImGuiWindow::eventUpdate(windowEvent);

	deviceManager_->eventUpdate(windowEvent);
}

void MainWindow::imGuiUpdate(const sdl::DeltaTime& deltaTime) {
	for (auto& subWindow : subWindows_) {
		subWindow->imGuiUpdate(deltaTime);
	}
	ioContext_.poll_one();
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
