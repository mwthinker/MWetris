#ifndef MWETRIS_NETWORK_DEBUGSERVER_H
#define MWETRIS_NETWORK_DEBUGSERVER_H

#include "protobufmessage.h"
#include "client.h"
#include "game/playerslot.h"
#include "debugclient.h"
#include <sdl/window.h>

#include "game/tetrisgame.h"

#include <mw/signal.h>

#include <memory>
#include <string>

namespace mwetris::network {

	struct ConnectedClient {
		std::string uuid;
		bool connected;
		bool allowToConnect;
	};

	enum class SlotType {
		Open,
		Remote,
		Closed
	};

	struct Slot {
		std::string clientUuid;
		std::string playerUuid;
		std::string name;
		bool ai;
		SlotType type;
	};

	class DebugServer : public std::enable_shared_from_this<DebugServer> {
	public:
		DebugServer();
		~DebugServer();

		void update(const sdl::DeltaTime& deltaTime);

		[[nodiscard]]
		std::shared_ptr<Client> createClient();

		void acquire(ProtobufMessage& message);

		void release(ProtobufMessage&& message);

		void sendPause(bool pause);
		
		bool isPaused() const;

		void restartGame();

		mw::signals::Connection addPlayerSlotsCallback(const std::function<void(const std::vector<Slot>&)>& playerSlots);

		mw::signals::Connection addInitGameCallback(const std::function<void(const game::InitGameEvent&)>& callback);

		mw::signals::Connection addClientListener(const std::function<void(const ConnectedClient&)>& callback);

		std::vector<ConnectedClient> getConnectedClients() const;

		void allowClientToConnect(const std::string& uuid, bool allowed);
		void disconnect(const std::string& uuid);
	
	private:
		class Impl;
		std::unique_ptr<Impl> impl_;
	};

}

#endif
