#ifndef MWETRIS_NETWORK_NETWORK_H
#define MWETRIS_NETWORK_NETWORK_H

#include "game/remoteplayer.h"

#include <thread>
#include <memory>
#include <string>

namespace mwetris::network {

	class Network {
	public:
		Network();

		[[nodiscard]] game::RemotePlayerPtr addRemotePlayer() {
			return nullptr;
		}

		void removeRemotePlayer(game::RemotePlayerPtr&& remotePlayer) {

		}

		const std::string& getServerId() const {
			return {serverId_};
		}

	private:
		void run();

		class Impl;
		std::unique_ptr<Impl> impl_;
		std::jthread thread_;
		std::string serverId_ = "sdfghjklzxcvbnm";
	};

}

#endif
