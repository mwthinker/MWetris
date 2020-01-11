#include "ui/tetriswindow.h"
#include "logger.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h> // or "../stdout_sinks.h" if no colors needed
#include <spdlog/sinks/basic_file_sink.h>

#include <sdl/initsdl.h>

int main(int, char**) {
	//auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	//console_sink->set_color(spdlog::level::level_enum::info, 0xffff0000);
	
	//auto console = spdlog::stdout_color_mt("MW");
	//spdlog::register_logger(console);
	//spdlog::get("MW");
		
	//spdlog::set_default_logger(console);
	//spdlog::set_automatic_registration(true);
	//spdlog::create (console_sink, "logger_name", "daylylog", 11, 59);

	try {
		const sdl::InitSdl SDL;
		tetris::TetrisWindow tetris;
		tetris.startLoop();
	} catch (std::exception&) {
		tetris::logger()->error("[Main] Fatal error has occured.");
	}

	return 0;
}


//  3.0       130       "#version 130"
//  3.1       140       "#version 140"
//  3.2       150       "#version 150"
//  3.3       330       "#version 330 core"
//  4.0       400       "#version 400 core"
//  4.1       410       "#version 410 core"
//  4.2       420       "#version 410 core"
//  4.3       430       "#version 430 core"