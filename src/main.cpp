#include "ui/tetriswindow.h"
#include "logger.h"
#include "configuration.h"

#include <sdl/initsdl.h>

int main(int, char**) {
	mwetris::logger::init();
	spdlog::info("[main] MWetris Version:  {}", PROJECT_VERSION);
	spdlog::info("[main] MWetris git hash:  {}", GIT_VERSION);

	const sdl::InitSdl SDL;
	mwetris::ui::TetrisWindow tetris;
	//tetris.setStartPage(tetris::ui::scene::Event::Play);
	tetris.startLoop();
	mwetris::Configuration::getInstance().quit();
	return 0;
}
