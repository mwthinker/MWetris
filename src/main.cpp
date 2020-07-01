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
		tetris::ui::TetrisWindow tetris;
		//tetris.setStartPage(tetris::ui::scene::Event::Play);
		tetris.startLoop();
		tetris::TetrisData::getInstance().quit();
	} catch (std::exception&) {
		spdlog::error("[Main] Fatal error has occured.");
	}

	return 0;
}
