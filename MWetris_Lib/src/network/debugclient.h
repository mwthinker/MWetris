#ifndef MWETRIS_NETWORK_DEBUGCLIENT_H
#define MWETRIS_NETWORK_DEBUGCLIENT_H

#include "protobufmessage.h"
#include "protobufmessagequeue.h"
#include "client.h"
#include "game/playerslot.h"
#include "debugserver.h"

#include "game/tetrisgame.h"

#include <asio.hpp>
#include <mw/signal.h>

#include <memory>
#include <string>
#include <queue>

namespace mwetris::network {

	class DebugClient : public Client {
	public:
		explicit DebugClient(std::shared_ptr<DebugServer> debugServer);

		~DebugClient() override;

		asio::awaitable<ProtobufMessage> receive() override;

		void send(ProtobufMessage&& message) override;

		void acquire(ProtobufMessage& message) override;

		void release(ProtobufMessage&& message) override;

		asio::io_context& getIoContext() override;

		// To be called by simulated server
		bool pollSentMessage(ProtobufMessage& message);

		// To be called by simulated server
		void pushReceivedMessage(ProtobufMessage&& message);

		// Used for testing
		void setSentToServerCallback(const std::function<void(const ProtobufMessage&)>& callback);
		
	private:
		std::queue<ProtobufMessage> receivedMessages_;
		std::queue<ProtobufMessage> sentMessages_;

		std::function<void(const ProtobufMessage&)> sendToServerCallback_;
		std::shared_ptr<DebugServer> debugServer_;
		asio::high_resolution_timer timer_;

	};

}

#endif
