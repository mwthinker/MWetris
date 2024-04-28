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

#include <memory>
#include <string>

namespace mwetris::network {

	class DebugServer : public std::enable_shared_from_this<DebugServer> {
	public:
		DebugServer();
		~DebugServer();

		void update(const sdl::DeltaTime& deltaTime);

		[[nodiscard]]
		std::shared_ptr<Client> createClient();

		void acquire(ProtobufMessage& message);

		void release(ProtobufMessage&& message);

		void sendPause(const std::string& clientUuuid, bool pause);
		
		bool isPaused(const std::string& clientUuuid) const;

		void restartGame(const std::string& clientUuuid);

		mw::signals::Connection addPlayerSlotsCallback(const std::function<void(const std::vector<Slot>&)>& playerSlots);

		mw::signals::Connection addInitGameCallback(const std::function<void(const game::InitGameEvent&)>& callback);

		mw::signals::Connection addClientListener(const std::function<void(const ConnectedClient&)>& callback);

		std::vector<ConnectedClient> getConnectedClients() const;

		void disconnect(const std::string& uuid);
	
	private:
		class Impl;
		std::unique_ptr<Impl> impl_;
	};

}

#endif
