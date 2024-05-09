#ifndef MWETRIS_NETWORK_TYPE_H
#define MWETRIS_NETWORK_TYPE_H

#include <fmt/core.h>
#include <fmt/format.h>
#include <string>
#include <functional>

namespace tp {
	
	class ClientId;
	class GameRoomId;

}

namespace mwetris::network {

	class ClientId {
	public:
		friend struct fmt::formatter<ClientId>;
		friend struct std::hash<mwetris::network::ClientId>;
		friend struct std::less<mwetris::network::ClientId>;
		friend struct fmt::formatter<mwetris::network::ClientId>;
		friend void setTp(const ClientId& clientId, tp::ClientId& tpClientId);

		static ClientId generateUniqueId();

		ClientId() = default;

		constexpr explicit ClientId(const std::string& id)
			: id_{id} {
		}
	
		// Implicit conversion from tp::ClientId to ClientId to simlyfy usage.
		ClientId(const tp::ClientId& tpClientId);

		friend bool operator==(const ClientId& lhs, const ClientId& rhs) {
			return lhs.id_ == rhs.id_;
		}

		friend bool operator!=(const ClientId& lhs, const ClientId& rhs) {
			return lhs.id_ != rhs.id_;
		}

		friend bool operator==(const tp::ClientId& lhs, const ClientId& rhs);
		friend bool operator==(const ClientId& lhs, const tp::ClientId& rhs);
		friend bool operator!=(const tp::ClientId& lhs, const ClientId& rhs);
		friend bool operator!=(const ClientId& lhs, const tp::ClientId& rhs);

		const char* c_str() const {
			return id_.c_str();
		}

	private:
		std::string id_;
	};

	class GameRoomId {
	public:
		friend struct fmt::formatter<GameRoomId>;
		friend struct std::hash<mwetris::network::GameRoomId>;
		friend struct std::less<mwetris::network::GameRoomId>;
		friend struct fmt::formatter<mwetris::network::GameRoomId>;
		friend void setTp(const GameRoomId& gameRoomId, tp::GameRoomId& tpGameRoomId);

		static GameRoomId generateUniqueId();

		GameRoomId() = default;

		constexpr explicit GameRoomId(const std::string& id)
			: id_{id} {}

		// Implicit conversion from tp::GameRoomId to GameRoomId to simlyfy usage.
		GameRoomId(const tp::GameRoomId& tpGameRoomId);
		GameRoomId& operator=(const tp::GameRoomId& tpGameRoomId);

		friend bool operator==(const GameRoomId& lhs, const GameRoomId& rhs) {
			return lhs.id_ == rhs.id_;
		}

		friend bool operator!=(const GameRoomId& lhs, const GameRoomId& rhs) {
			return lhs.id_ != rhs.id_;
		}

		operator bool() const {
			return id_.empty();
		}

		friend bool operator==(const tp::GameRoomId& lhs, const GameRoomId& rhs);
		friend bool operator==(const GameRoomId& lhs, const tp::GameRoomId& rhs);
		friend bool operator!=(const tp::GameRoomId& lhs, const GameRoomId& rhs);
		friend bool operator!=(const GameRoomId& lhs, const tp::GameRoomId& rhs);

		const char* c_str() const {
			return id_.c_str();
		}

	private:
		std::string id_;
	};

}

// ClientId

template <> struct fmt::formatter<mwetris::network::ClientId> : fmt::formatter<std::string_view> {
	auto format(const mwetris::network::ClientId& clientId, fmt::format_context& ctx) const {
		return formatter<string_view>::format(clientId.id_, ctx);
	}
};

template <> struct fmt::formatter<tp::ClientId> : fmt::formatter<mwetris::network::ClientId> {
	auto format(const mwetris::network::ClientId& c, fmt::format_context& ctx) const {
		return formatter<mwetris::network::ClientId>::format(c, ctx);
	}
};

template <>
struct std::hash<mwetris::network::ClientId> {
	inline size_t operator()(const mwetris::network::ClientId& clientId) const {
		return std::hash<std::string_view>{}(clientId.id_);
	}
};

template<> struct std::less<mwetris::network::ClientId> {
	bool operator() (const mwetris::network::ClientId& lhs, const mwetris::network::ClientId& rhs) const {
		return lhs.id_ < rhs.id_;
	}
};

// GameRoomId

template <> struct fmt::formatter<mwetris::network::GameRoomId> : fmt::formatter<std::string_view> {
	auto format(const mwetris::network::GameRoomId& clientId, fmt::format_context& ctx) const {
		return formatter<string_view>::format(clientId.id_, ctx);
	}
};

template <> struct fmt::formatter<tp::GameRoomId> : fmt::formatter<mwetris::network::GameRoomId> {
	auto format(const mwetris::network::GameRoomId& c, fmt::format_context& ctx) const {
		return formatter<mwetris::network::GameRoomId>::format(c, ctx);
	}
};

template <>
struct std::hash<mwetris::network::GameRoomId> {
	inline size_t operator()(const mwetris::network::GameRoomId& clientId) const {
		return std::hash<std::string_view>{}(clientId.id_);
	}
};

template<> struct std::less<mwetris::network::GameRoomId> {
	bool operator() (const mwetris::network::GameRoomId& lhs, const mwetris::network::GameRoomId& rhs) const {
		return lhs.id_ < rhs.id_;
	}
};

#endif
