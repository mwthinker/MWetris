#include "id.h"

#include <protocol/shared.pb.h>

namespace mwetris::network {

	namespace {

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

	// GameRoomId

	ClientId ClientId::generateUniqueId() {
		return ClientId{generateUuid()};
	}

	ClientId::ClientId(const tp::ClientId& tpClientId)
		: id_{tpClientId.id()} {
	}

	void fromCppToProto(const ClientId& clientId, tp::ClientId& tpClientId) {
		tpClientId.set_id(clientId.id_);
	}

	bool operator==(const tp::ClientId& lhs, const ClientId& rhs) {
		return lhs.id() == rhs.id_;
	}

	bool operator==(const ClientId& lhs, const tp::ClientId& rhs) {
		return lhs.id_ == rhs.id();
	}

	bool operator!=(const tp::ClientId& lhs, const ClientId& rhs) {
		return lhs.id() != rhs.id_;
	}

	bool operator!=(const ClientId& lhs, const tp::ClientId& rhs) {
		return lhs.id_ != rhs.id();
	}

	// GameRoomId

	GameRoomId GameRoomId::generateUniqueId() {
		return GameRoomId{generateUuid()};
	}

	GameRoomId::GameRoomId(const tp::GameRoomId& tpGameRoomId)
		: id_{tpGameRoomId.id()} {}

	GameRoomId& GameRoomId::operator=(const tp::GameRoomId& tpGameRoomId) {
		id_ = tpGameRoomId.id();
		return *this;
	}

	void fromCppToProto(const GameRoomId& gameRoomId, tp::GameRoomId& tpGameRoomId) {
		tpGameRoomId.set_id(gameRoomId.id_);
	}

	bool operator==(const tp::GameRoomId& lhs, const GameRoomId& rhs) {
		return lhs.id() == rhs.id_;
	}

	bool operator==(const GameRoomId& lhs, const tp::GameRoomId& rhs) {
		return lhs.id_ == rhs.id();
	}

	bool operator!=(const tp::GameRoomId& lhs, const GameRoomId& rhs) {
		return lhs.id() != rhs.id_;
	}

	bool operator!=(const GameRoomId& lhs, const tp::GameRoomId& rhs) {
		return lhs.id_ != rhs.id();
	}

	// PlayerId

	PlayerId PlayerId::generateUniqueId() {
		return PlayerId{generateUuid()};
	}

	PlayerId::PlayerId(const tp::PlayerId& tpPlayerId)
		: id_{tpPlayerId.id()} {}

	PlayerId& PlayerId::operator=(const tp::PlayerId& tpPlayerId) {
		id_ = tpPlayerId.id();
		return *this;
	}

	void fromCppToProto(const PlayerId& gameRoomId, tp::PlayerId& tpPlayerId) {
		tpPlayerId.set_id(gameRoomId.id_);
	}

	bool operator==(const tp::PlayerId& lhs, const PlayerId& rhs) {
		return lhs.id() == rhs.id_;
	}

	bool operator==(const PlayerId& lhs, const tp::PlayerId& rhs) {
		return lhs.id_ == rhs.id();
	}

	bool operator!=(const tp::PlayerId& lhs, const PlayerId& rhs) {
		return lhs.id() != rhs.id_;
	}

	bool operator!=(const PlayerId& lhs, const tp::PlayerId& rhs) {
		return lhs.id_ != rhs.id();
	}
}
