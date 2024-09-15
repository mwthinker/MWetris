#ifndef MWETRIS_NETWORK_SERVER_H
#define MWETRIS_NETWORK_SERVER_H

#include "id.h"

#include <google/protobuf/message_lite.h>

namespace network {

	struct ConnectedClient {
		ClientId clientId;
	};

	enum class SlotType {
		Open,
		Remote,
		Closed
	};

	struct Slot {
		ClientId clientId;
		PlayerId playerId;
		std::string name;
		bool ai;
		SlotType type;
	};

	class Server {
	public:
		virtual ~Server() = default;
		
		virtual void sendToClient(const ClientId& clientId, const google::protobuf::MessageLite& message) = 0;

		virtual void triggerConnectedClientEvent(const ConnectedClient& connectedClient) = 0;

		virtual void triggerPlayerSlotEvent(const std::vector<Slot>& slots) = 0;
	};

}

#endif
