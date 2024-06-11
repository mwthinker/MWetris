#ifndef MWETRIS_NETWORK_ASIO_H
#define MWETRIS_NETWORK_ASIO_H

#include <asio.hpp>
#include <fmt/format.h>

template <> struct fmt::formatter<asio::ip::tcp::endpoint> : fmt::formatter<std::string_view> {
	auto format(const asio::ip::tcp::endpoint& endpoint, fmt::format_context& ctx) const {
		return formatter<string_view>::format(fmt::format("{}:{}", endpoint.address().to_string(), endpoint.port()), ctx);
	}
};

#endif
