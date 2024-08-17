#ifndef MWETRIS_NETWORK_TCPCLIENT_H
#define MWETRIS_NETWORK_TCPCLIENT_H

#include "client.h"
#include "asio.h"
#include "protobufmessage.h"
#include "protobufmessagequeue.h"

#include "../game/playerslot.h"
#include "../game/tetrisgame.h"

#include <mw/signal.h>

#include <memory>
#include <string>
#include <queue>

namespace mwetris::network {

	class TcpClient : public Client, public std::enable_shared_from_this<TcpClient> {
	public:
		/// @brief Connect to server.
		/// @param ioContext to use for asynchronous operations.
		static std::shared_ptr<TcpClient> connectToServer(asio::io_context& ioContext, const std::string& ip, int port);

		/// @brief Use exisiting connection on active socket.
		/// @param ioContext to use for asynchronous operations.
		/// @param socket that is connectd to server.
		static std::shared_ptr<TcpClient> useExistingSocket(asio::io_context& ioContext, asio::ip::tcp::socket socket);

		~TcpClient() override;

		asio::awaitable<ProtobufMessage> receive() override;

		void send(ProtobufMessage&& message) override;

		void acquire(ProtobufMessage& message) override;

		void release(ProtobufMessage&& message) override;

		asio::io_context& getIoContext() override;

		const std::string& getName() const;

		void stop() override;

	private:
		TcpClient(asio::io_context& ioContext, const std::string& ip, int port);

		TcpClient(asio::io_context& ioContext, asio::ip::tcp::socket socket);

		asio::awaitable<ProtobufMessage> asyncRead();

		asio::awaitable<void> waitForConnection();

		/// @brief Keeps this tcp client alive until the async operation is done.
		/// @param client to act on
		/// @return coroutine handle.
		static asio::awaitable<ProtobufMessage> receive(std::shared_ptr<TcpClient> client);

		asio::io_context& ioContext_;
		asio::high_resolution_timer tryToConnectTimer_, waitingToConnect_;
		asio::ip::tcp::socket socket_;
		ProtobufMessageQueue queue_;
		std::string name_;
		bool isStopped_ = false;
		bool connected_ = false;
	};

}

#endif
