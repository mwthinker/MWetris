#include "debugserver.h"
#include "debugclient.h"
#include "protobufmessagequeue.h"
#include "util.h"
#include "util/uuid.h"
#include "game/player.h"
#include "gameroom.h"
#include "server.h"
#include "util/uuid.h"
#include "id.h"
#include "tcpclient.h"


#include <helper.h>

#include <server_to_client.pb.h>
#include <client_to_server.pb.h>

#include <spdlog/spdlog.h>
#include <asio.hpp>

#include <queue>
#include <variant>

namespace mwetris::network {

	DebugServer::DebugServer(asio::io_context& ioContext)
		: ServerCore{ioContext} {
	}

	DebugServer::~DebugServer() {
	}

	asio::awaitable<void> DebugServer::run() {
		for (auto& remote : remotes_) {
			asio::co_spawn(ioContext_, [this, rm = remote]() mutable -> asio::awaitable<void> {
				//co_await remote.client->receive();
				co_await receivedFromClient(rm);
			}, asio::detached);
		}
		co_return;
	}

	/*
	asio::awaitable<void> listener() {
		auto executor = co_await asio::this_coro::executor;
		asio::ip::tcp::acceptor acceptor(executor, {asio::ip::tcp::v4(), 55555});
		for (;;) {
			asio::ip::tcp::socket socket = co_await acceptor.async_accept(asio::use_awaitable);
			asio::co_spawn(executor, receivedFromClient(std::move(socket)), asio::detached);
		}
	}

	asio::awaitable<void> receivedFromClient(asio::ip::tcp::socket&& socket) {
		auto client = std::make_shared<TcpClient>(ioContext_);
		const auto& remote = remotes_.emplace_back(Remote{
			.client = nullptr,
			.clientId = ClientId::generateUniqueId()
		});
		triggerConnectedClient(remote);

		ProtobufMessage protobufMessage;
		while (socket.is_open()) {
			protobufMessage.clear();

			// Read header.
			std::size_t size = co_await socket.async_read_some(
				asio::buffer(protobufMessage.getData(), protobufMessage.getHeaderSize()),
				asio::use_awaitable
			);
			protobufMessage.reserveBodySize();

			// Read body.
			size = co_await socket.async_read_some(
				asio::buffer(protobufMessage.getBodyData(), protobufMessage.getBodySize()),
				asio::use_awaitable
			);
		}
		co_return;
	}
	*/

	std::shared_ptr<Client> DebugServer::addClient() {
		auto client = DebugClientOnServer::create(shared_from_this());
		const auto& remote = remotes_.emplace_back(Remote{
			.client = client,
			.clientId = ClientId::generateUniqueId()
		});
		triggerConnectedClient(remote);
		return client->getDebugClientOnNetwork();
	}

	void DebugServer::release(ProtobufMessage&& message) {
		messageQueue_.release(std::move(message));
	}

	void DebugServer::acquire(ProtobufMessage& message) {
		messageQueue_.acquire(message);
	}
		
	asio::awaitable<ProtobufMessage> DebugServer::receive() {
		ProtobufMessage protobufMessage;
		messageQueue_.acquire(protobufMessage);
		protobufMessage.clear();
		co_return protobufMessage;
	}

	void DebugServer::send(ProtobufMessage&& message) {
	}

}
