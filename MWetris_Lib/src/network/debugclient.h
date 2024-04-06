#ifndef MWETRIS_NETWORK_DEBUGCLIENT_H
#define MWETRIS_NETWORK_DEBUGCLIENT_H

#include "protobufmessage.h"
#include "protobufmessagequeue.h"
#include "client.h"
#include "game/playerslot.h"

#include "game/tetrisgame.h"

#include <mw/signal.h>

#include <memory>
#include <string>
#include <queue>

namespace mwetris::network {

	class DebugClient : public Client {
	public:
		friend class DebugServer;

		DebugClient(std::shared_ptr<DebugServer> debugServer);

		~DebugClient() override;

		bool receive(ProtobufMessage& message) override;

		void send(ProtobufMessage&& message) override;

		void acquire(ProtobufMessage& message) override;

		void release(ProtobufMessage&& message) override;

		// To be called by simulated server
		bool pollSentMessage(ProtobufMessage& message);

		// To be called by simulated server
		void pushReceivedMessage(ProtobufMessage&& message);

		const std::string& getUuid() const;

		void setUuid(const std::string& uuid) {
			uuid_ = uuid;
		}
		
	private:
		std::queue<ProtobufMessage> receivedMessages_;
		std::queue<ProtobufMessage> sentMessages_;

		std::shared_ptr<DebugServer> debugServer_;
		std::string uuid_;
	};

}

#endif
