#ifndef MWETRIS_UTIL_AUXILIARY_H
#define MWETRIS_UTIL_AUXILIARY_H

#include <chrono>

namespace network {
	
	template<typename>
	inline constexpr bool always_false_v = false;

	constexpr double toSeconds(const auto& duration) {
		return std::chrono::duration<double>(duration).count();
	}

}

#endif
