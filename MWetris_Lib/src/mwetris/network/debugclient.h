#ifndef MWETRIS_NETWORK_DEBUGCLIENT_H
#define MWETRIS_NETWORK_DEBUGCLIENT_H

#include "protobufmessage.h"
#include "protobufmessagequeue.h"
#include "client.h"
#include "asio.h"

#include <mw/signal.h>

#include <memory>
#include <string>
#include <queue>
#include <functional>

namespace mwetris::network {

	class DebugServer;
	class DebugClient;
	class DebugClientOnNetwork;

	class DebugClientOnServer : public Client, public std::enable_shared_from_this<DebugClientOnServer> {
	public:
		static std::shared_ptr<DebugClientOnServer> create(std::shared_ptr<DebugServer> debugServer);

		~DebugClientOnServer() override;

		void stop() override;

		asio::awaitable<ProtobufMessage> receive() override;

		void send(ProtobufMessage&& message) override;

		void acquire(ProtobufMessage& message) override;

		void release(ProtobufMessage&& message) override;

		asio::io_context& getIoContext() override;

		std::shared_ptr<DebugClientOnNetwork> getDebugClientOnNetwork();

		void pushReceivedMessage(ProtobufMessage&& message);

		bool isConnected() const override;

		void reconnect() override;

	private:
		explicit DebugClientOnServer(std::shared_ptr<DebugServer> debugServer);

		asio::high_resolution_timer timer_;
		std::shared_ptr<DebugServer> debugServer_;
		std::shared_ptr<DebugClientOnNetwork> debugClientOnNetwork_;
		std::queue<ProtobufMessage> receivedMessages_;
	};

	class DebugClientOnNetwork : public Client {
	public:
		static std::shared_ptr<DebugClientOnNetwork> create(std::shared_ptr<DebugClientOnServer> debugClientOnServer);

		~DebugClientOnNetwork() override;

		void stop() override;

		asio::awaitable<ProtobufMessage> receive() override;

		void send(ProtobufMessage&& message) override;

		void acquire(ProtobufMessage& message) override;

		void release(ProtobufMessage&& message) override;

		void pushReceivedMessage(ProtobufMessage&& message);

		asio::io_context& getIoContext() override;

		void setSentToServerCallback(const std::function<void(const ProtobufMessage&)>& callback);

		bool isConnected() const override;

		void reconnect() override;

	private:
		friend class DebugClientOnServer;
		explicit DebugClientOnNetwork(std::weak_ptr<DebugClientOnServer> debugClientOnServer);

		asio::high_resolution_timer timer_;
		std::weak_ptr<DebugClientOnServer> debugClientOnServer_;
		std::queue<ProtobufMessage> receivedMessages_;
		std::function<void(const ProtobufMessage&)> sendToServerCallback_;
	};

}

#endif
