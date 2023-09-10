#ifndef MWETRIS_NETWORK_DEBUGSERVER_H
#define MWETRIS_NETWORK_DEBUGSERVER_H

#include "protobufmessage.h"
#include "client.h"
#include "game/playerslot.h"
#include "debugclient.h"

#include "game/tetrisgame.h"

#include <mw/signal.h>

#include <memory>
#include <string>

namespace mwetris::network {

	class DebugServer {
	public:
		DebugServer();
		~DebugServer();

		std::shared_ptr<Client> createClient();

	};

}

#endif
