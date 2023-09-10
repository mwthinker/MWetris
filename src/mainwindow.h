#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <sdl/imguiwindow.h>

#include <ui/tetriswindow.h>
#include <ui/networkdebugwindow.h>

class MainWindow : public sdl::ImGuiWindow {
public:
	MainWindow();

	~MainWindow();
	
private:
	void initPreLoop() override;

	void eventUpdate(const SDL_Event& windowEvent) override;

	void imGuiUpdate(const sdl::DeltaTime& deltaTime) override;

	void imGuiEventUpdate(const SDL_Event& windowEvent) override;

	std::unique_ptr<mwetris::ui::TetrisWindow> tetrisWindow_;
	std::vector<std::unique_ptr<mwetris::ui::SubWindow>> subWindows_;
};

#endif
