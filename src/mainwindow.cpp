#include "mainwindow.h"

#include <mwetris/configuration.h>

#include <mwetris/ui/networkdebugwindow.h>
#include <mwetris/ui/tetriswindow.h>
#include <mwetris/network/debugclient.h>
#include <mwetris/network/network.h>

#include <network/tcpclient.h>
#include <network/tcpserver.h>

#include <mwetris/ui/tetriswindow.h>

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
	if (server_) {
		server_->stop();
	}
	ioContext_.stop();
	ioContext_.run();
	mwetris::Configuration::getInstance().quit();
}

void MainWindow::initPreLoop() {
	sdl::ImGuiWindow::initPreLoop();

	ImGui::GetIO().Fonts->AddFontFromFileTTF("fonts/Ubuntu-B.ttf", 12); // Used by demo window.
	
	mwetris::Configuration::getInstance().getImGuiButtonFont();
	mwetris::Configuration::getInstance().getImGuiDefaultFont();
	mwetris::Configuration::getInstance().getImGuiHeaderFont();

	deviceManager_ = std::make_shared<mwetris::game::DeviceManager>();
	

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
		subWindows_.push_back(std::make_unique<mwetris::ui::NetworkDebugWindow>(server_));
	}
}

void MainWindow::initSingleTcpClient() {
	auto [ip, port] = mwetris::Configuration::getInstance().getNetwork().server;
	for (int i = 0; i < config_.windows; ++i) {
		auto tcpClient = mwetris::network::TcpClient::connectToServer(ioContext_, ip, port);
		auto type = (i == 0) ? mwetris::ui::TetrisWindow::Type::MainWindow : mwetris::ui::TetrisWindow::Type::SecondaryWindow;
		std::string name = (i == 0) ? "MainWindow" : fmt::format("SecondaryWindow{}", i);
		subWindows_.push_back(std::make_unique<mwetris::ui::TetrisWindow>(name, type, *this,
			deviceManager_,
			std::make_shared<mwetris::network::Network>(tcpClient)
		));
	}
}

void MainWindow::initTcpServer() {
	auto [ip, port] = mwetris::Configuration::getInstance().getNetwork().server;
	auto settings = mwetris::network::TcpServer::Settings{
		.port = port
	};
	auto server = std::make_shared<mwetris::network::TcpServer>(ioContext_, settings);
	for (int i = 0; i < config_.windows; ++i) {
		auto tcpClient = mwetris::network::TcpClient::connectToServer(ioContext_, ip, port);
		auto type = (i == 0) ? mwetris::ui::TetrisWindow::Type::MainWindow : mwetris::ui::TetrisWindow::Type::SecondaryWindow;
		std::string name = (i == 0) ? "MainWindow" : fmt::format("SecondaryWindow{}", i);
		subWindows_.push_back(std::make_unique<mwetris::ui::TetrisWindow>(name, type, *this,
			deviceManager_,
			std::make_shared<mwetris::network::Network>(tcpClient)
		));
	}
	server_ = server;
	server_->start();
}

void MainWindow::initDebugServer() {
	auto server = std::make_shared<mwetris::network::DebugServer>(ioContext_);
	for (int i = 0; i < config_.windows; ++i) {
		auto debugClient = server->addClient();
		auto type = (i == 0) ? mwetris::ui::TetrisWindow::Type::MainWindow : mwetris::ui::TetrisWindow::Type::SecondaryWindow;
		std::string name = (i == 0) ? "MainWindow" : fmt::format("SecondaryWindow{}", i);
		subWindows_.push_back(std::make_unique<mwetris::ui::TetrisWindow>(name, type, *this,
			deviceManager_,
			std::make_shared<mwetris::network::Network>(debugClient)
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
