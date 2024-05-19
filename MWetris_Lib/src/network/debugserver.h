#ifndef MWETRIS_NETWORK_DEBUGSERVER_H
#define MWETRIS_NETWORK_DEBUGSERVER_H

#include "protobufmessage.h"
#include "client.h"
#include "game/playerslot.h"
#include "client.h"
#include "server.h"
#include "game/tetrisgame.h"

#include <sdl/window.h>
#include <mw/signal.h>

#include <asio.hpp>

#include <memory>
#include <string>

namespace mwetris::network {

	class DebugServer : public std::enable_shared_from_this<DebugServer> {
	public:
		explicit DebugServer(asio::io_context& ioContext);
		~DebugServer();

		void update(const sdl::DeltaTime& deltaTime);

		[[nodiscard]]
		std::shared_ptr<Client> createClient();

		void acquire(ProtobufMessage& message);

		void release(ProtobufMessage&& message);

		void sendPause(const GameRoomId& gameRoomId, bool pause);
		
		bool isPaused(const GameRoomId& gameRoomId) const;

		void restartGame(const GameRoomId& gameRoomId);

		mw::signals::Connection addPlayerSlotsCallback(const std::function<void(const std::vector<Slot>&)>& playerSlots);

		mw::signals::Connection addInitGameCallback(const std::function<void(const game::InitGameEvent&)>& callback);

		mw::signals::Connection addClientListener(const std::function<void(const ConnectedClient&)>& callback);

		std::vector<ConnectedClient> getConnectedClients() const;

		void disconnect(const GameRoomId& gameRoomId);

		asio::io_context& getIoContext();
	
	private:
		class Impl;
		std::unique_ptr<Impl> impl_;
	};

}

#endif
