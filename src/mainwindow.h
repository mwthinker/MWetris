#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <sdl/imguiwindow.h>

#include <ui/subwindow.h>
#include <game/devicemanager.h>

#include <network/debugserver.h>

class MainWindow : public sdl::ImGuiWindow {
public:
	MainWindow();

	~MainWindow();
	
private:
	void initPreLoop() override;

	void eventUpdate(const SDL_Event& windowEvent) override;

	void imGuiUpdate(const sdl::DeltaTime& deltaTime) override;

	void imGuiEventUpdate(const SDL_Event& windowEvent) override;

	std::vector<std::unique_ptr<mwetris::ui::SubWindow>> subWindows_;
	std::shared_ptr<mwetris::game::DeviceManager> deviceManager_;
	std::shared_ptr<mwetris::network::DebugServer> debugServer_;
};

#endif
