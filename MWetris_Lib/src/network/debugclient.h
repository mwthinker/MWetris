#ifndef MWETRIS_NETWORK_DEBUGCLIENT_H
#define MWETRIS_NETWORK_DEBUGCLIENT_H

#include "protobufmessage.h"
#include "client.h"
#include "game/playerslot.h"

#include <string>
#include <memory>

#include <mw/signal.h>

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

		mw::signals::Connection addPlayerSlotsCallback(const std::function<void(const std::vector<game::PlayerSlot>&)>& playerSlots);

	private:
		class Impl;
		std::unique_ptr<Impl> impl_;
	};

}

#endif
