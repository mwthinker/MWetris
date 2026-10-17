#include "mainwindow.h"

#include <app/configuration.h>

#include <app/ui/networkdebugwindow.h>
#include <app/ui/tetriswindow.h>
#include <app/cnetwork/network.h>

#include <network/tcpclient.h>
#include <network/tcpserver.h>

#include <chrono>

using namespace std::chrono_literals;

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
	subWindows_.clear();
	deviceManager_.reset();
	controlProfiles_.reset();
	app::Configuration::getInstance().quit();
}

void MainWindow::preLoop() {
	auto& configuration = app::Configuration::getInstance();
	applyCommittedSettings(configuration.getEditableSettings());
	auto font = configuration.getEditableSettings().assets.windowFont;
	ImGui::GetIO().Fonts->AddFontFromFileTTF(font.c_str(), 12); // Used by demo window.
	
	configuration.getImGuiButtonFont();
	configuration.getImGuiDefaultFont();
	configuration.getImGuiHeaderFont();
	configuration.getImGuiBoardFont();

	configuration.init(gpuDevice_);
	connections_ += configuration.settingsChanged.connect([this](const auto& previous, const auto& current) {
		applyCommittedSettings(current, &previous);
	});

	controlProfiles_ = std::make_shared<app::ControlProfiles>(
		configuration.loadControlProfiles()
			.or_else([](const std::string& error) -> app::ControlProfilesLoadResult {
				spdlog::error("Could not load control profiles: {}", error);
				return app::ControlProfiles::createDefaultState();
			})
			.value(),
		[](const app::ControlProfilesState& state) {
			return app::Configuration::getInstance().persistControlProfiles(state);
		});
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

void MainWindow::applyCommittedSettings(const app::Configuration::EditableSettings& settings, const app::Configuration::EditableSettings* previous) {
	if (!window_) {
		return;
	}
	if (!previous || previous->window.resizable != settings.window.resizable) {
		SDL_SetWindowResizable(window_, settings.window.resizable);
	}
	if (!previous || previous->window.bordered != settings.window.bordered) {
		SDL_SetWindowBordered(window_, settings.window.bordered);
	}
	if (!previous || previous->window.minWidth != settings.window.minWidth || previous->window.minHeight != settings.window.minHeight) {
		SDL_SetWindowMinimumSize(window_, settings.window.minWidth, settings.window.minHeight);
	}
	if (!previous || previous->window.width != settings.window.width || previous->window.height != settings.window.height) {
		SDL_SetWindowSize(window_, settings.window.width, settings.window.height);
	}
	if (!previous || previous->window.maximized != settings.window.maximized) {
		if (settings.window.maximized) {
			SDL_MaximizeWindow(window_);
		} else if ((SDL_GetWindowFlags(window_) & SDL_WINDOW_MAXIMIZED) != 0) {
			SDL_RestoreWindow(window_);
		}
	}
	if (!previous || previous->window.limitFps != settings.window.limitFps) {
		setLoopSleepingTime(settings.window.limitFps ? 16ms : 0ms);
	}
	if (!previous && gpuDevice_) {
		auto presentMode = SDL_GPU_PRESENTMODE_VSYNC;
		if (!settings.window.vsync) {
			if (SDL_WindowSupportsGPUPresentMode(gpuDevice_, window_, SDL_GPU_PRESENTMODE_IMMEDIATE)) {
				presentMode = SDL_GPU_PRESENTMODE_IMMEDIATE;
			} else if (SDL_WindowSupportsGPUPresentMode(gpuDevice_, window_, SDL_GPU_PRESENTMODE_MAILBOX)) {
				presentMode = SDL_GPU_PRESENTMODE_MAILBOX;
			} else {
				spdlog::warn("VSync cannot be disabled on this GPU/window.");
			}
		}
		if (!SDL_SetGPUSwapchainParameters(gpuDevice_, window_, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, presentMode)) {
			spdlog::warn("Could not configure swapchain presentation: {}", SDL_GetError());
		}
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
			controlProfiles_,
			std::make_shared<app::cnetwork::Network>(tcpClient, controlProfiles_)
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
			controlProfiles_,
			std::make_shared<app::cnetwork::Network>(tcpClient, controlProfiles_)
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
			controlProfiles_,
			std::make_shared<app::cnetwork::Network>(debugClient, controlProfiles_)
		));
	}
	server_ = server;
	server_->start();
}

void MainWindow::renderImGui(const sdl::DeltaTime& deltaTime) {
	for (auto& subWindow : subWindows_) {
		subWindow->imGuiUpdate(deltaTime);
	}
	ioContext_.poll();
	deviceManager_->tick();
}

void MainWindow::processEvent(const SDL_Event& windowEvent) {
	deviceManager_->eventUpdate(windowEvent);

	switch (windowEvent.type) {
		case SDL_EVENT_WINDOW_FOCUS_LOST:
			if (app::Configuration::getInstance().isWindowPauseOnLostFocus()) {
				for (auto& subWindow : subWindows_) {
					if (auto* tetrisWindow = dynamic_cast<app::ui::TetrisWindow*>(subWindow.get())) {
						tetrisWindow->pauseOnFocusLost();
					}
				}
			}
			break;
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			if (windowEvent.button.button == SDL_BUTTON_LEFT && windowEvent.button.clicks >= 2
				&& app::Configuration::getInstance().isFullscreenOnDoubleClick()) {
				auto fullscreen = (SDL_GetWindowFlags(window_) & SDL_WINDOW_FULLSCREEN) != 0;
				SDL_SetWindowFullscreen(window_, !fullscreen);
			} else if (windowEvent.button.button == SDL_BUTTON_LEFT
				&& windowEvent.button.y <= app::Configuration::getInstance().getWindowBarHeight()
				&& app::Configuration::getInstance().isMoveWindowByHoldingDownMouse()) {
				draggingWindow_ = true;
				SDL_GetGlobalMouseState(&dragMouseX_, &dragMouseY_);
				SDL_GetWindowPosition(window_, &dragWindowX_, &dragWindowY_);
				SDL_CaptureMouse(true);
			}
			break;
		case SDL_EVENT_MOUSE_MOTION:
			if (draggingWindow_) {
				float mouseX = 0.f;
				float mouseY = 0.f;
				SDL_GetGlobalMouseState(&mouseX, &mouseY);
				SDL_SetWindowPosition(window_, dragWindowX_ + static_cast<int>(mouseX - dragMouseX_), dragWindowY_ + static_cast<int>(mouseY - dragMouseY_));
			}
			break;
		case SDL_EVENT_MOUSE_BUTTON_UP:
			if (windowEvent.button.button == SDL_BUTTON_LEFT && draggingWindow_) {
				draggingWindow_ = false;
				SDL_CaptureMouse(false);
			}
			break;
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
			quit();
			break;
		case SDL_EVENT_KEY_DOWN:
			switch (windowEvent.key.key) {
				case SDLK_ESCAPE:
					if (!ImGui::GetIO().WantCaptureKeyboard) {
						quit();
					}
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
