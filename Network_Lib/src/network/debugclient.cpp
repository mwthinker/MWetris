#include "debugclient.h"
#include "debugserver.h"

#include <network/protobufmessagequeue.h>
#include <tetris/helper.h>

#include <spdlog/spdlog.h>

#include <queue>

namespace mwetris::network {

	DebugClientOnServer::DebugClientOnServer(std::shared_ptr<DebugServer> debugServer)
		: timer_{debugServer->getIoContext()}
		, debugServer_{debugServer} {

		timer_.expires_after(std::chrono::seconds{0});
	}

	std::shared_ptr<DebugClientOnServer> DebugClientOnServer::create(std::shared_ptr<DebugServer> debugServer) {
		return std::shared_ptr<DebugClientOnServer>(new DebugClientOnServer{debugServer});
	}

	DebugClientOnServer::~DebugClientOnServer() {
	}

	void DebugClientOnServer::stop() {
	}

	asio::awaitable<ProtobufMessage> DebugClientOnServer::receive() {
		while (receivedMessages_.empty()) {
			co_await timer_.async_wait(asio::use_awaitable);
			timer_.expires_after(std::chrono::seconds{0});
		}
		auto message = std::move(receivedMessages_.front());
		receivedMessages_.pop();
		co_return message;
	}

	void DebugClientOnServer::send(ProtobufMessage&& message) {
		debugClientOnNetwork_->pushReceivedMessage(std::move(message));
	}

	void DebugClientOnServer::acquire(ProtobufMessage& message) {
		debugServer_->acquire(message);
	}

	void DebugClientOnServer::release(ProtobufMessage&& message) {
		debugServer_->release(std::move(message));
	}

	asio::io_context& DebugClientOnServer::getIoContext() {
		return debugServer_->getIoContext();
	}

	std::shared_ptr<DebugClientOnNetwork> DebugClientOnServer::getDebugClientOnNetwork() {
		if (!debugClientOnNetwork_) {
			debugClientOnNetwork_ = DebugClientOnNetwork::create(shared_from_this());
		}
		return debugClientOnNetwork_;
	}

	void DebugClientOnServer::pushReceivedMessage(ProtobufMessage&& message) {
		receivedMessages_.push(std::move(message));
	}

	bool DebugClientOnServer::isConnected() const {
		return true;
	}

	void DebugClientOnServer::reconnect() {
	}

	// ------------------------------------------------------------------------

	std::shared_ptr<DebugClientOnNetwork> DebugClientOnNetwork::create(std::shared_ptr<DebugClientOnServer> debugClientOnServer) {
		return std::shared_ptr<DebugClientOnNetwork>(new DebugClientOnNetwork{debugClientOnServer});
	}

	DebugClientOnNetwork::DebugClientOnNetwork(std::weak_ptr<DebugClientOnServer> debugClientOnServer)
		: timer_{debugClientOnServer.lock()->getIoContext()}
		, debugClientOnServer_{debugClientOnServer} {

		timer_.expires_after(std::chrono::seconds{0});
	}

	DebugClientOnNetwork::~DebugClientOnNetwork() {
	}

	void DebugClientOnNetwork::stop() {
	}

	asio::awaitable<ProtobufMessage> DebugClientOnNetwork::receive() {
		while (receivedMessages_.empty()) {
			co_await timer_.async_wait(asio::use_awaitable);
			timer_.expires_after(std::chrono::seconds(0));
		}
		auto message = std::move(receivedMessages_.front());
		receivedMessages_.pop();
		co_return message;
	}

	void DebugClientOnNetwork::send(ProtobufMessage&& message) {
		if (sendToServerCallback_) {
			sendToServerCallback_(message);
		}
		debugClientOnServer_.lock()->pushReceivedMessage(std::move(message));
	}

	void DebugClientOnNetwork::acquire(ProtobufMessage& message) {
		debugClientOnServer_.lock()->acquire(message);
	}

	void DebugClientOnNetwork::release(ProtobufMessage&& message) {
		debugClientOnServer_.lock()->release(std::move(message));
	}

	void DebugClientOnNetwork::pushReceivedMessage(ProtobufMessage&& message) {
		receivedMessages_.push(std::move(message));
	}

	asio::io_context& DebugClientOnNetwork::getIoContext() {
		return debugClientOnServer_.lock()->getIoContext();
	}

	void DebugClientOnNetwork::setSentToServerCallback(const std::function<void(const ProtobufMessage&)>& callback) {
		sendToServerCallback_ = callback;
	}

	bool DebugClientOnNetwork::isConnected() const {
		return true;
	}

	void DebugClientOnNetwork::reconnect() {}

}
