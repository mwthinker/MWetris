#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <sdl/window.h>

#include <app/ui/subwindow.h>
#include <app/game/devicemanager.h>
#include <app/configuration.h>
#include <network/debugserver.h>

#include <network/servercore.h>
#include <mw/signal.h>

namespace app {

	class ControlProfiles;

}

enum class Network {
	SingleTcpClient,
	TcpServer,
	DebugServer
};

class MainWindow : public sdl::Window {
public:
	struct Config {
		int windows = 1;
		bool showDebugWindow = false;
		bool showDemoWindow = false;
		Network network = Network::SingleTcpClient;
	};

	explicit MainWindow(const Config& config);

	~MainWindow() override;
	
private:
	void preLoop() override;

	void initSingleTcpClient();

	void initTcpServer();

	void initDebugServer();

	void processEvent(const SDL_Event& windowEvent) override;
	void processWindowMouseActions();
	void applyCommittedSettings(const app::Configuration::EditableSettings& settings, const app::Configuration::EditableSettings* previous = nullptr);

	void renderImGui(const sdl::DeltaTime& deltaTime) override;
		
	asio::io_context ioContext_; // Must be declared first to guarantee destruction order.
	std::shared_ptr<app::ControlProfiles> controlProfiles_;
	std::shared_ptr<app::game::DeviceManager> deviceManager_;
	std::vector<std::unique_ptr<app::ui::SubWindow>> subWindows_;
	std::shared_ptr<network::ServerCore> server_;
	mw::signals::ScopedConnections connections_;
	Config config_;
	bool draggingWindow_ = false;
	float dragMouseX_ = 0.f;
	float dragMouseY_ = 0.f;
	int dragWindowX_ = 0;
	int dragWindowY_ = 0;
};

#endif
