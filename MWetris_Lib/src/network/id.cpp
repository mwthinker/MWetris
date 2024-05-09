#include "id.h"

#include "util/uuid.h"
#include "shared.pb.h"

namespace mwetris::network {

	// GameRoomId

	ClientId ClientId::generateUniqueId() {
		return ClientId{util::generateUuid()};
	}

	ClientId::ClientId(const tp::ClientId& tpClientId)
		: id_(tpClientId.id()) {
	}

	void setTp(const ClientId& clientId, tp::ClientId& tpClientId) {
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

	void setTp(const GameRoomId& gameRoomId, tp::GameRoomId& tpGameRoomId) {
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
}
