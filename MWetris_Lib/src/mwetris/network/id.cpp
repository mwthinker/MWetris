#include "id.h"

#include "util/uuid.h"

#include <protocol/shared.pb.h>

namespace mwetris::network {

	// GameRoomId

	ClientId ClientId::generateUniqueId() {
		return ClientId{util::generateUuid()};
	}

	ClientId::ClientId(const tp::ClientId& tpClientId)
		: id_(tpClientId.id()) {
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
		return GameRoomId{util::generateUuid()};
	}

	GameRoomId::GameRoomId(const tp::GameRoomId& tpGameRoomId)
		: id_(tpGameRoomId.id()) {}

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
		return PlayerId{util::generateUuid()};
	}

	PlayerId::PlayerId(const tp::PlayerId& tpPlayerId)
		: id_(tpPlayerId.id()) {}

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
