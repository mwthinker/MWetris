#ifndef MWETRIS_NETWORK_CLIENT_H
#define MWETRIS_NETWORK_CLIENT_H

#include "protobufmessage.h"

#include <asio.hpp>

namespace mwetris::network {

	class Client {
	public:
		virtual ~Client() = default;

		virtual asio::awaitable<ProtobufMessage> receive() = 0;

		virtual void send(ProtobufMessage&& message) = 0;

		virtual void acquire(ProtobufMessage& message) = 0;

		virtual void release(ProtobufMessage&& message) = 0;

		virtual asio::io_context& getIoContext() = 0;
	};

}

#endif
