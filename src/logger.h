#ifndef MWETRIS_SDL_LOGGER_H
#define MWETRIS_SDL_LOGGER_H

#include <spdlog/spdlog.h>

namespace tetris::logger {
	
	bool init(const std::string& folderPath = "");

} // Namespace sdl.

#endif // MWETRIS_SDL_LOGGER_H
