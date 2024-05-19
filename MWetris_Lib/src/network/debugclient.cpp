#include "debugclient.h"
#include "protobufmessagequeue.h"
#include "util.h"
#include "game/player.h"
#include "debugserver.h"

#include <helper.h>

#include <spdlog/spdlog.h>

#include <queue>

namespace mwetris::network {

	DebugClient::DebugClient(std::shared_ptr<DebugServer> debugServer)
		: debugServer_{debugServer}
		, timer_{debugServer->getIoContext()} {

		timer_.expires_after(std::chrono::seconds{0});
	}

	DebugClient::~DebugClient() = default;

	asio::awaitable<ProtobufMessage> DebugClient::receive() {
		while (receivedMessages_.empty()) {
			co_await timer_.async_wait(asio::use_awaitable);
			timer_.expires_after(std::chrono::seconds(0));
		}
		auto message = std::move(receivedMessages_.front());
		receivedMessages_.pop();
		co_return message;
	}

	void DebugClient::send(ProtobufMessage&& message) {
		if (sendToServerCallback_) {
			sendToServerCallback_(message);
		}
		sentMessages_.push(std::move(message));
	}

	void DebugClient::acquire(ProtobufMessage& message) {
		debugServer_->acquire(message);
	}

	void DebugClient::release(ProtobufMessage&& message) {
		debugServer_->release(std::move(message));
	}

	asio::io_context& DebugClient::getIoContext() {
		return debugServer_->getIoContext();
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

	void DebugClient::setSentToServerCallback(const std::function<void(const ProtobufMessage&)>& callback) {
		sendToServerCallback_ = callback;
	}

}
