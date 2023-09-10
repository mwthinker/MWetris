#include "protobufmessage.h"
#include "debugclient.h"
#include "protobufmessagequeue.h"
#include "util.h"
#include "game/player.h"
#include "game/remoteplayer.h"

#include <helper.h>

#include <message.pb.h>

#include <spdlog/spdlog.h>

#include <queue>

namespace mwetris::network {

	DebugServer::DebugServer() {

	}

	DebugServer::~DebugServer() {

	}

	std::shared_ptr<Client> DebugServer::createDebugClient() {
		return nullptr;
	}

}
