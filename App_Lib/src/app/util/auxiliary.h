#ifndef APP_UTIL_AUXILIARY_H
#define APP_UTIL_AUXILIARY_H

#include <chrono>

namespace app::util {
	
	template<class>
	inline constexpr bool always_false_v = false;

	constexpr double toSeconds(const auto& duration) {
		return std::chrono::duration<double>(duration).count();
	}

}

#endif
