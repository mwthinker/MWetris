#ifndef MWETRIS_NETWORK_CLIENT_H
#define MWETRIS_NETWORK_CLIENT_H

#include "protobufmessage.h"

#include <asio.hpp>

namespace network {

	class Client {
	public:
		virtual ~Client() = default;
		
		/// @brief Stop any active coroutines.
		/// 
		/// I.e. makes the coroutine drop the ownership of this object. But current
		/// jobs must still be run to completion. I.e. ioContext.poll() must be
		/// called until all jobs are done to garantee that the coroutine is finished.
		virtual void stop() = 0;

		/// @brief Receive a message from the server.
		/// @return The received message.
		virtual asio::awaitable<ProtobufMessage> receive() = 0;

		/// @brief Send a message to the server.
		/// @param message
		virtual void send(ProtobufMessage&& message) = 0;

		/// @brief Acquire a message from memory.
		/// Used in order to avoid unnecessary memory allocations.
		/// @param message
		virtual void acquire(ProtobufMessage& message) = 0;

		/// @brief Return memory to the client.
		/// Used in order to avoid unnecessary memory allocations.
		/// @param message
		virtual void release(ProtobufMessage&& message) = 0;

		/// @brief Get the io_context.
		virtual asio::io_context& getIoContext() = 0;


		/// @brief Is the client connected.
		virtual bool isConnected() const = 0;

		/// @brief Reconnect the client. Only affects sockets connecting to the server 
		/// (not the other way around).
		virtual void reconnect() = 0;
	};

}

#endif
