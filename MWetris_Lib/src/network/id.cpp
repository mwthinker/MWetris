#include "id.h"

#include "util/uuid.h"
#include "shared.pb.h"

namespace mwetris::network {

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

}
