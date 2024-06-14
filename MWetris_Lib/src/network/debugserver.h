#ifndef MWETRIS_NETWORK_DEBUGSERVER_H
#define MWETRIS_NETWORK_DEBUGSERVER_H

#include "protobufmessage.h"
#include "client.h"
#include "servercore.h"

#include <sdl/window.h>
#include <mw/signal.h>

#include <asio.hpp>

#include <memory>
#include <string>

namespace mwetris::network {

	class DebugServer : public ServerCore, public std::enable_shared_from_this<DebugServer> {
	public:
		explicit DebugServer(asio::io_context& ioContext);
		~DebugServer() override;

		asio::awaitable<void> run() override;

		[[nodiscard]]
		std::shared_ptr<Client> addClient();

		void sendPause(const GameRoomId& gameRoomId, bool pause);
		
		bool isPaused(const GameRoomId& gameRoomId) const;

		void restartGame(const GameRoomId& gameRoomId);

		void disconnect(const GameRoomId& gameRoomId);

		asio::awaitable<ProtobufMessage> receive();

		void send(ProtobufMessage&& message);

		void acquire(ProtobufMessage& message);

		void release(ProtobufMessage&& message);
	};

}

#endif
