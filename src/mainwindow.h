#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <sdl/imguiwindow.h>

#include <ui/subwindow.h>
#include <game/devicemanager.h>

#include <network/debugserver.h>
#include <network/servercore.h>
#include <asio.hpp>

enum class Network {
	SingleTcpClient,
	TcpServer,
	DebugServer
};

class MainWindow : public sdl::ImGuiWindow {
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
	void initPreLoop() override;

	void initSingleTcpClient();

	void initTcpServer();

	void initDebugServer();

	void eventUpdate(const SDL_Event& windowEvent) override;

	void imGuiUpdate(const sdl::DeltaTime& deltaTime) override;

	void imGuiEventUpdate(const SDL_Event& windowEvent) override;
		
	asio::io_context ioContext_; // Must be declared first to guarantee destruction order.
	std::vector<std::unique_ptr<mwetris::ui::SubWindow>> subWindows_;
	std::shared_ptr<mwetris::game::DeviceManager> deviceManager_;
	std::shared_ptr<mwetris::network::ServerCore> server_;
	Config config_;
};

#endif
