#ifndef MWETRIS_SDL_LOGGER_H
#define MWETRIS_SDL_LOGGER_H

#include <sdl/logger.h>

namespace tetris {
	
	static std::shared_ptr<spdlog::logger> logger() {
		return sdl::logger("tetris");
	}

} // Namespace sdl.

#endif // MWETRIS_SDL_LOGGER_H
