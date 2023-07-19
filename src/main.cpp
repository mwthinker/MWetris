#include "ui/tetriswindow.h"
#include "logger.h"
#include "configuration.h"

#include <sdl/initsdl.h>
#include <sdl/gamecontroller.h>

int main(int argc, char** argv) {
	mwetris::logger::init();
	spdlog::info("[main] MWetris Version:  {}", PROJECT_VERSION);
	spdlog::info("[main] MWetris git hash:  {}", GIT_VERSION);
	spdlog::info("[main] MWetris GUI:  {}", GUID);

	const sdl::InitSdl SDL{SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER};
	mwetris::ui::TetrisWindow tetrisWindow;

	sdl::GameController::loadGameControllerMappings("gamecontrollerdb.txt");

	if (argc == 2) {
		std::string flag{argv[1]};
		if (flag == "-p" || flag == "--page") {
			//tetrisWindow.setStartPage(mwetris::ui::scene::Event::Play);
		}
	}

	tetrisWindow.startLoop();
	return 0;
}
