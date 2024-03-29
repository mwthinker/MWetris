#include "debugclient.h"
#include "protobufmessagequeue.h"
#include "util.h"
#include "game/player.h"
#include "game/remoteplayer.h"
#include "debugserver.h"

#include <helper.h>

#include <spdlog/spdlog.h>

#include <queue>
#include <random>

namespace mwetris::network {

	namespace {

		constexpr std::string_view Characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

		std::string generateUuid() {
			static std::mt19937 generator{std::random_device{}()};
			static std::uniform_int_distribution<> distribution{0, static_cast<int>(Characters.size() - 1)};

			constexpr int UniqueIdSize = 16;
			std::string unique(UniqueIdSize, 'X');

			for (auto& key : unique) {
				key = Characters[distribution(generator)];
			}

			return unique;
		}

	}

	DebugClient::DebugClient(std::shared_ptr<DebugServer> debugServer) 
		: debugServer_{debugServer} {

		uuid_ = generateUuid();
	}

	DebugClient::~DebugClient() = default;

	const std::string& DebugClient::getUuid() const{
		return uuid_;
	}

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
