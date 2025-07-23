#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <sdl/window.h>

#include <app/ui/subwindow.h>
#include <app/game/devicemanager.h>
#include <network/debugserver.h>

#include <network/servercore.h>

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

	void renderImGui(const sdl::DeltaTime& deltaTime) override;
		
	asio::io_context ioContext_; // Must be declared first to guarantee destruction order.
	std::vector<std::unique_ptr<app::ui::SubWindow>> subWindows_;
	std::shared_ptr<app::game::DeviceManager> deviceManager_;
	std::shared_ptr<network::ServerCore> server_;
	Config config_;
};

#endif
