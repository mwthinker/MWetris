#include "debugclient.h"
#include "protobufmessagequeue.h"
#include "util.h"
#include "game/player.h"
#include "game/remoteplayer.h"
#include "debugserver.h"

#include <helper.h>

#include <message.pb.h>

#include <spdlog/spdlog.h>

#include <queue>

namespace tp = tetris_protocol;

namespace mwetris::network {

	DebugClient::DebugClient(std::shared_ptr<DebugServer> debugServer) 
		: debugServer_{debugServer} {
	}

	DebugClient::~DebugClient() = default;

	bool DebugClient::receive(ProtobufMessage& message) {
		if (receivedMessages_.empty()) {
			return false;
		}
		message = std::move(receivedMessages_.front());
		receivedMessages_.pop();
		return true;
	}

	void DebugClient::send(ProtobufMessage&& message) {
		sentMessages_.push(std::move(message));
	}

	void DebugClient::acquire(ProtobufMessage& message) {
		debugServer_->acquire(message);
	}

	void DebugClient::release(ProtobufMessage&& message) {
		debugServer_->release(std::move(message));
	}

	bool DebugClient::pollSentMessage(ProtobufMessage& message) {
		if (sentMessages_.empty()) {
			return false;
		}
		message = std::move(sentMessages_.front());
		sentMessages_.pop();
		return true;
	}

	void DebugClient::pushReceivedMessage(ProtobufMessage&& message) {
		receivedMessages_.push(std::move(message));
	}

}
