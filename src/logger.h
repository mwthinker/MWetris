#ifndef MWETRIS_LOGGER_H
#define MWETRIS_LOGGER_H

#include <spdlog/spdlog.h>

namespace mwetris::logger {
	
	bool init(const std::string& folderPath = "");

}

#endif
