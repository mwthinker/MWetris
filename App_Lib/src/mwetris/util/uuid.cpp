#include "uuid.h"
#include <random>

namespace mwetris::util {

	constexpr std::string_view Characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

	std::string generateUuid() {
		static std::mt19937 generator{std::random_device{}()};
		static std::uniform_int_distribution<> distribution{0, static_cast<int>(Characters.size() - 1)};

		constexpr int UniqueIdSize = 16;
		std::string unique(UniqueIdSize, 'X');

		for (auto& key : unique) {
			key = Characters[distribution(generator)];
		}

		return unique;
	}

}
