#ifndef MWETRIS_NETWORK_TCPCLIENT_H
#define MWETRIS_NETWORK_TCPCLIENT_H

#include "protobufmessage.h"
#include "protobufmessagequeue.h"
#include "client.h"
#include "game/playerslot.h"

#include "game/tetrisgame.h"

#include <asio.hpp>
#include <mw/signal.h>

#include <memory>
#include <string>
#include <queue>

namespace mwetris::network {

	class TcpClient : public Client {
	public:
		/// @brief Connect to server.
		/// @param ioContext The io_context to use for asynchronous operations.
		TcpClient(asio::io_context& ioContext, const std::string& ip, int port);

		/// @brief Use exisiting connection on active socket.
		/// @param ioContext The io_context to use for asynchronous operations.
		/// @param socket The socket to use.
		TcpClient(asio::io_context& ioContext, asio::ip::tcp::socket socket);

		~TcpClient() override;

		asio::awaitable<ProtobufMessage> receive() override;

		void send(ProtobufMessage&& message) override;

		void acquire(ProtobufMessage& message) override;

		void release(ProtobufMessage&& message) override;

		asio::io_context& getIoContext() override;
		
	private:
		asio::io_context& ioContext_;
		asio::high_resolution_timer timer_;
		asio::ip::tcp::socket socket_;
		ProtobufMessageQueue queue_;
		std::string name_;
	};

}

#endif
