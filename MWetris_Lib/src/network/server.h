#ifndef MWETRIS_NETWORK_SERVER_H
#define MWETRIS_NETWORK_SERVER_H

#include "game/tetrisgameevent.h"

#include <google/protobuf/message_lite.h>

namespace mwetris::network {

	struct ConnectedClient {
		std::string uuid;
	};

	enum class SlotType {
		Open,
		Remote,
		Closed
	};

	struct Slot {
		std::string clientUuid;
		std::string playerUuid;
		std::string name;
		bool ai;
		SlotType type;
	};

	class Server {
	public:
		virtual ~Server() = default;
		
		virtual void sendToClient(const std::string& clientUuid, const google::protobuf::MessageLite& message) = 0;

		virtual void triggerConnectedClientEvent(const ConnectedClient& connectedClient) = 0;

		virtual void triggerPlayerSlotEvent(const std::vector<Slot>& slots) = 0;

		virtual void triggerInitGameEvent(const game::InitGameEvent& initGameEvent) = 0;
	};

}

#endif
