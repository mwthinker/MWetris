#include "mainwindow.h"

#include <app/configuration.h>

#include <app/ui/networkdebugwindow.h>
#include <app/ui/tetriswindow.h>
#include <app/cnetwork/network.h>

#include <network/tcpclient.h>
#include <network/tcpserver.h>

MainWindow::MainWindow(const Config& config) 
	: config_{config} {

	setPosition(app::Configuration::getInstance().getWindowPositionX(), app::Configuration::getInstance().getWindowPositionY());
	setSize(app::Configuration::getInstance().getWindowWidth(), app::Configuration::getInstance().getWindowHeight());
	setTitle("MWetris");
	setIcon(app::Configuration::getInstance().getWindowIcon());
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

void MainWindow::preLoop() {
	SDL_SetWindowResizable(window_, true);
	SDL_SetWindowBordered(window_, app::Configuration::getInstance().isWindowBordered());
	ImGui::GetIO().Fonts->AddFontFromFileTTF("fonts/Ubuntu-B.ttf", 12); // Used by demo window.
	
	app::Configuration::getInstance().getImGuiButtonFont();
	app::Configuration::getInstance().getImGuiDefaultFont();
	app::Configuration::getInstance().getImGuiHeaderFont();

	app::Configuration::getInstance().init(gpuContext_);

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

void MainWindow::renderImGui(const sdl::DeltaTime& deltaTime) {
	for (auto& subWindow : subWindows_) {
		subWindow->imGuiUpdate(deltaTime);
	}
	ioContext_.poll_one();
	deviceManager_->tick();
}

void MainWindow::processEvent(const SDL_Event& windowEvent) {
	deviceManager_->eventUpdate(windowEvent);

	switch (windowEvent.type) {
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
			quit();
			break;
		case SDL_EVENT_KEY_DOWN:
			switch (windowEvent.key.key) {
				case SDLK_ESCAPE:
					quit();
					break;
			}
			break;
		case SDL_EVENT_QUIT:
			quit();
			break;
	}

	for (auto& subWindow : subWindows_) {
		subWindow->imGuiEventUpdate(windowEvent);
	}
}
