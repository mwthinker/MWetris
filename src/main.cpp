#include "ui/tetriswindow.h"
#include "logger.h"
#include "tetrisdata.h"

#include <sdl/initsdl.h>

int main(int, char**) {
	try {
		tetris::logger::init();
		spdlog::info("[main] MWetris Version:  {}", PROJECT_VERSION);
		spdlog::info("[main] MWetris git hash:  {}", GIT_VERSION);

		const sdl::InitSdl SDL;
		tetris::TetrisWindow tetris;
		//tetris.setStartPage(tetris::TetrisWindow::Page::PLAY);
		tetris.startLoop();
		tetris::TetrisData::getInstance().quit();
	} catch (std::exception&) {
		spdlog::error("[Main] Fatal error has occured.");
	}

	return 0;
}
