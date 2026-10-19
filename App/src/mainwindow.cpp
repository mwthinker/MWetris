#include "mainwindow.h"

#include <app/configuration.h>

#include <app/ui/networkdebugwindow.h>
#include <app/ui/tetriswindow.h>
#include <app/cnetwork/network.h>

#include <network/tcpclient.h>
#include <network/tcpserver.h>

#include <algorithm>
#include <array>
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
	connections_.clear();
	subWindows_.clear();
	server_.reset();
	deviceManager_.reset();
	controlProfiles_.reset();

	ioContext_.restart();
	ioContext_.poll();

	app::Configuration::getInstance().resetRuntimeResources();
}

MainWindow::ExitAction MainWindow::getExitAction() const {
	return exitAction_;
}

void MainWindow::requestRestart() {
	exitAction_ = ExitAction::Restart;
	quit();
}

void MainWindow::preLoop() {
	auto& configuration = app::Configuration::getInstance();
	baseImGuiStyle_ = ImGui::GetStyle();
	applyCommittedSettings(configuration.getEditableSettings());
	configuration.initImGuiFonts();

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
	if (!previous || previous->window.uiScale != settings.window.uiScale) {
		userUiScale_ = settings.window.uiScale;
		uiScaleDirty_ = true;
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

void MainWindow::applyUiScale() {
	auto monitorScale = SDL_GetWindowDisplayScale(window_);
	if (monitorScale <= 0.f) {
		spdlog::warn("Could not determine monitor DPI scale: {}", SDL_GetError());
		monitorScale = 1.f;
	}
	const auto effectiveScale = monitorScale * userUiScale_;

	auto& style = ImGui::GetStyle();
	std::array<ImVec4, ImGuiCol_COUNT> colors;
	std::copy_n(style.Colors, ImGuiCol_COUNT, colors.begin());
	style = baseImGuiStyle_;
	std::copy_n(colors.begin(), ImGuiCol_COUNT, style.Colors);
	style.ScaleAllSizes(effectiveScale);
	style.FontScaleMain = effectiveScale;
	uiScaleDirty_ = false;
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
			std::make_shared<app::cnetwork::Network>(tcpClient, controlProfiles_),
			[this]() { requestRestart(); }
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
			std::make_shared<app::cnetwork::Network>(tcpClient, controlProfiles_),
			[this]() { requestRestart(); }
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
			std::make_shared<app::cnetwork::Network>(debugClient, controlProfiles_),
			[this]() { requestRestart(); }
		));
	}
	server_ = server;
	server_->start();
}

void MainWindow::renderImGui(const sdl::DeltaTime& deltaTime) {
	if (uiScaleDirty_) {
		applyUiScale();
	}
	for (auto& subWindow : subWindows_) {
		subWindow->imGuiUpdate(deltaTime);
	}
	// ImGui-captured mouse events do not reach processEvent(), so handle window mouse actions here.
	processWindowMouseActions();
	ioContext_.poll();
	deviceManager_->tick();
}

void MainWindow::processWindowMouseActions() {
	auto& configuration = app::Configuration::getInstance();
	if (configuration.isFullscreenOnDoubleClick() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
		draggingWindow_ = false;
		SDL_CaptureMouse(false);
		auto fullscreen = (SDL_GetWindowFlags(window_) & SDL_WINDOW_FULLSCREEN) != 0;
		if (!SDL_SetWindowFullscreen(window_, !fullscreen)) {
			spdlog::warn("Could not toggle fullscreen: {}", SDL_GetError());
		}
		return;
	}

	if (configuration.isMoveWindowByHoldingDownMouse()
		&& ImGui::IsMouseClicked(ImGuiMouseButton_Left)
		&& !ImGui::IsAnyItemActive()) {
		draggingWindow_ = true;
		SDL_GetGlobalMouseState(&dragMouseX_, &dragMouseY_);
		SDL_GetWindowPosition(window_, &dragWindowX_, &dragWindowY_);
		SDL_CaptureMouse(true);
	}

	if (draggingWindow_ && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
		float mouseX = 0.f;
		float mouseY = 0.f;
		SDL_GetGlobalMouseState(&mouseX, &mouseY);
		SDL_SetWindowPosition(window_, dragWindowX_ + static_cast<int>(mouseX - dragMouseX_), dragWindowY_ + static_cast<int>(mouseY - dragMouseY_));
	} else if (draggingWindow_) {
		draggingWindow_ = false;
		SDL_CaptureMouse(false);
	}
}

void MainWindow::processEvent(const SDL_Event& windowEvent) {
	deviceManager_->eventUpdate(windowEvent);

	switch (windowEvent.type) {
		case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
			if (windowEvent.window.windowID == SDL_GetWindowID(window_)) {
				uiScaleDirty_ = true;
			}
			break;
		case SDL_EVENT_WINDOW_FOCUS_LOST:
			if (app::Configuration::getInstance().isWindowPauseOnLostFocus()) {
				for (auto& subWindow : subWindows_) {
					if (auto* tetrisWindow = dynamic_cast<app::ui::TetrisWindow*>(subWindow.get())) {
						tetrisWindow->pauseOnFocusLost();
					}
				}
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
