#include "logger.h"
#include "mainwindow.h"

#include <app/ui/tetriswindow.h>
#include <app/configuration.h>

#include <sdl/gamecontroller.h>

#include <argparse/argparse.hpp>
#include <fmt/printf.h>
#include <fmt/ostream.h>

template <> struct fmt::formatter<argparse::ArgumentParser> : fmt::ostream_formatter {};

void startApplication(MainWindow::Config& config) {
	mwetris::logger::init();
	spdlog::info("[main] MWetris Version:  {}", PROJECT_VERSION);
	spdlog::info("[main] MWetris git hash:  {}  date: {}", GIT_VERSION, GIT_DATE);
	spdlog::info("[main] MWetris vcpkg hash:  {}  date: {}", VCPKG_HASH, VCPKG_DATE);
	spdlog::info("[main] MWetris GUI:  {}", MWETRIS_GUID);

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
	auto& group = program.add_mutually_exclusive_group(false);
	group.add_argument("-s", "--simulate")
		.help("Simulate network")
		.flag();
	group.add_argument("-t", "--tcpclient")
		.help("TCP client (default choice)")
		.default_value(true)
		.flag();
	group.add_argument("-T", "--tcpserver")
		.help("TCP server")
		.flag();
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
	if (program.get<bool>("-s")) {
		config.network = Network::DebugServer;
	} else if (program.get<bool>("-t")) {
		config.network = Network::SingleTcpClient;
	} else if (program.get<bool>("-T")) {
		config.network = Network::TcpServer;
	}

	startApplication(config);
	return 0;
}
