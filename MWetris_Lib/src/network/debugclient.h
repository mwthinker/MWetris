#ifndef MWETRIS_NETWORK_DEBUGCLIENT_H
#define MWETRIS_NETWORK_DEBUGCLIENT_H

#include "protobufmessage.h"
#include "client.h"
#include "game/playerslot.h"

#include "game/tetrisgame.h"

#include <mw/signal.h>

#include <memory>
#include <string>

namespace mwetris::network {

	class DebugClient : public Client {
	public:
		DebugClient();

		~DebugClient() override;

		bool receive(ProtobufMessage& message) override;

		void send(ProtobufMessage&& message) override;

		void acquire(ProtobufMessage& message) override;

		void release(ProtobufMessage&& message) override;

		void connect(const std::string& uuid);

		void disconnect(const std::string& uuid);

		void sendPause(bool pause);
		bool isPaused() const;

		void restartGame();

		mw::signals::Connection addPlayerSlotsCallback(const std::function<void(const std::vector<game::PlayerSlot>&)>& playerSlots);

		mw::signals::Connection addInitGameCallback(const std::function<void(const game::InitGameEvent&)>& callback);

	private:
		class Impl;
		std::unique_ptr<Impl> impl_;
	};

	class DebugServer {
	public:
		std::shared_ptr<DebugClient> createDebugClient();

	};

}

#endif
