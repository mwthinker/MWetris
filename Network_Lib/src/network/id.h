#ifndef MWETRIS_NETWORK_TYPE_H
#define MWETRIS_NETWORK_TYPE_H

#include <fmt/core.h>
#include <fmt/format.h>
#include <string>
#include <functional>

namespace tp {
	
	class ClientId;
	class GameRoomId;
	class PlayerId;

}

namespace network {

	class ClientId {
	public:
		friend struct fmt::formatter<ClientId>;
		friend struct std::hash<network::ClientId>;
		friend struct std::less<network::ClientId>;
		friend struct fmt::formatter<network::ClientId>;
		friend void fromCppToProto(const ClientId& clientId, tp::ClientId& tpClientId);

		static ClientId generateUniqueId();

		ClientId() = default;

		explicit ClientId(const std::string& id)
			: id_{id} {
		}
	
		// Implicit conversion from tp::ClientId to ClientId to simlify usage.
		ClientId(const tp::ClientId& tpClientId);

		friend bool operator==(const ClientId& lhs, const ClientId& rhs) {
			return lhs.id_ == rhs.id_;
		}

		friend bool operator!=(const ClientId& lhs, const ClientId& rhs) {
			return lhs.id_ != rhs.id_;
		}

		explicit operator bool() const {
			return !id_.empty();
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

	struct GameRoomClient {
		ClientId clientId;
		int connectionId;
	};

	class GameRoomId {
	public:
		friend struct fmt::formatter<GameRoomId>;
		friend struct std::hash<network::GameRoomId>;
		friend struct std::less<network::GameRoomId>;
		friend struct fmt::formatter<network::GameRoomId>;
		friend void fromCppToProto(const GameRoomId& gameRoomId, tp::GameRoomId& tpGameRoomId);

		static GameRoomId generateUniqueId();

		GameRoomId() = default;

		explicit GameRoomId(const std::string& id)
			: id_{id} {
		}

		// Implicit conversion from tp::GameRoomId to GameRoomId to simlify usage.
		GameRoomId(const tp::GameRoomId& tpGameRoomId);
		GameRoomId& operator=(const tp::GameRoomId& tpGameRoomId);

		friend bool operator==(const GameRoomId& lhs, const GameRoomId& rhs) {
			return lhs.id_ == rhs.id_;
		}

		friend bool operator!=(const GameRoomId& lhs, const GameRoomId& rhs) {
			return lhs.id_ != rhs.id_;
		}

		explicit operator bool() const {
			return !id_.empty();
		}

		friend bool operator==(const tp::GameRoomId& lhs, const GameRoomId& rhs);
		friend bool operator==(const GameRoomId& lhs, const tp::GameRoomId& rhs);
		friend bool operator!=(const tp::GameRoomId& lhs, const GameRoomId& rhs);
		friend bool operator!=(const GameRoomId& lhs, const tp::GameRoomId& rhs);

		const char* c_str() const {
			return id_.c_str();
		}

		bool isEmpty() const {
			return id_.empty();
		}

	private:
		std::string id_;
	};

	class PlayerId {
	public:
		friend struct fmt::formatter<PlayerId>;
		friend struct std::hash<network::PlayerId>;
		friend struct std::less<network::PlayerId>;
		friend struct fmt::formatter<network::PlayerId>;
		friend void fromCppToProto(const PlayerId& gameRoomId, tp::PlayerId& tpPlayerId);

		static PlayerId generateUniqueId();

		PlayerId() = default;

		explicit PlayerId(const std::string& id)
			: id_{id} {}

		// Implicit conversion from tp::PlayerId to PlayerId to simlify usage.
		PlayerId(const tp::PlayerId& tpPlayerId);
		PlayerId& operator=(const tp::PlayerId& tpPlayerId);

		friend bool operator==(const PlayerId& lhs, const PlayerId& rhs) {
			return lhs.id_ == rhs.id_;
		}

		friend bool operator!=(const PlayerId& lhs, const PlayerId& rhs) {
			return lhs.id_ != rhs.id_;
		}

		explicit operator bool() const {
			return !id_.empty();
		}

		friend bool operator==(const tp::PlayerId& lhs, const PlayerId& rhs);
		friend bool operator==(const PlayerId& lhs, const tp::PlayerId& rhs);
		friend bool operator!=(const tp::PlayerId& lhs, const PlayerId& rhs);
		friend bool operator!=(const PlayerId& lhs, const tp::PlayerId& rhs);

		const char* c_str() const {
			return id_.c_str();
		}

	private:
		std::string id_;
	};

}

// ClientId

template <> struct fmt::formatter<network::ClientId> : fmt::formatter<std::string_view> {
	auto format(const network::ClientId& clientId, fmt::format_context& ctx) const {
		return formatter<string_view>::format(clientId.id_, ctx);
	}
};

template <> struct fmt::formatter<tp::ClientId> : fmt::formatter<network::ClientId> {
	auto format(const network::ClientId& c, fmt::format_context& ctx) const {
		return formatter<network::ClientId>::format(c, ctx);
	}
};

template <>
struct std::hash<network::ClientId> {
	inline size_t operator()(const network::ClientId& clientId) const {
		return std::hash<std::string_view>{}(clientId.id_);
	}
};

template<> struct std::less<network::ClientId> {
	bool operator() (const network::ClientId& lhs, const network::ClientId& rhs) const {
		return lhs.id_ < rhs.id_;
	}
};

// GameRoomId

template <> struct fmt::formatter<network::GameRoomId> : fmt::formatter<std::string_view> {
	auto format(const network::GameRoomId& clientId, fmt::format_context& ctx) const {
		return formatter<string_view>::format(clientId.id_, ctx);
	}
};

template <> struct fmt::formatter<tp::GameRoomId> : fmt::formatter<network::GameRoomId> {
	auto format(const network::GameRoomId& c, fmt::format_context& ctx) const {
		return formatter<network::GameRoomId>::format(c, ctx);
	}
};

template <>
struct std::hash<network::GameRoomId> {
	inline size_t operator()(const network::GameRoomId& clientId) const {
		return std::hash<std::string_view>{}(clientId.id_);
	}
};

template<> struct std::less<network::GameRoomId> {
	bool operator() (const network::GameRoomId& lhs, const network::GameRoomId& rhs) const {
		return lhs.id_ < rhs.id_;
	}
};

// PlayerId

template <> struct fmt::formatter<network::PlayerId> : fmt::formatter<std::string_view> {
	auto format(const network::PlayerId& clientId, fmt::format_context& ctx) const {
		return formatter<string_view>::format(clientId.id_, ctx);
	}
};

template <> struct fmt::formatter<tp::PlayerId> : fmt::formatter<network::PlayerId> {
	auto format(const network::PlayerId& c, fmt::format_context& ctx) const {
		return formatter<network::PlayerId>::format(c, ctx);
	}
};

template <>
struct std::hash<network::PlayerId> {
	inline size_t operator()(const network::PlayerId& clientId) const {
		return std::hash<std::string_view>{}(clientId.id_);
	}
};

template<> struct std::less<network::PlayerId> {
	bool operator() (const network::PlayerId& lhs, const network::PlayerId& rhs) const {
		return lhs.id_ < rhs.id_;
	}
};

#endif
