#include "ui/tetriswindow.h"
#include "logger.h"
#include "configuration.h"

#include "mainwindow.h"

#include <sdl/initsdl.h>
#include <sdl/gamecontroller.h>

#include <argparse/argparse.hpp>
#include <fmt/printf.h>
#include <fmt/ostream.h>

template <> struct fmt::formatter<argparse::ArgumentParser> : fmt::ostream_formatter {};

void startApplication(MainWindow::Config& config) {
	mwetris::logger::init();
	spdlog::info("[main] MWetris Version:  {}", PROJECT_VERSION);
	spdlog::info("[main] MWetris git hash:  {}", GIT_VERSION);
	spdlog::info("[main] MWetris GUI:  {}", MWETRIS_GUID);

	const sdl::InitSdl SDL{SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER};
	MainWindow mainWindow{config};

	sdl::GameController::loadGameControllerMappings("gamecontrollerdb.txt");

	mainWindow.startLoop();
}

int main(int argc, char** argv) {
	argparse::ArgumentParser program{"MWetris", PROJECT_VERSION};
	program.add_description("A Tetris game.");
	program
		.add_argument("-w", "--windows")
		.help("Number of windows")
		.default_value(1)
		.scan<'i', int>();
	program
		.add_argument("-d", "--debug")
		.help("Show debug windows")
		.flag();
	program
		.add_argument("-D", "--demo")
		.help("Show demo window")
		.flag();

	try {
		program.parse_args(argc, argv);
	} catch (const std::exception& err) {
		fmt::println("Error: {}", err.what());
		fmt::println("{}", program);
		return 1;
	}

	MainWindow::Config config;
	config.windows = program.get<int>("-w");
	config.showDebugWindow = program.get<bool>("-d");
	config.showDemoWindow = program.get<bool>("-D");

	startApplication(config);
	return 0;
}
