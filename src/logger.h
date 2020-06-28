#ifndef MWETRIS_LOGGER_H
#define MWETRIS_LOGGER_H

#include <spdlog/spdlog.h>

namespace tetris::logger {
	
	bool init(const std::string& folderPath = "");

}

#endif
