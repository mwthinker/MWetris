#include "tcpserver.h"
#include "protobufmessagequeue.h"
#include "util.h"

#include "game/player.h"

#include <tetris/helper.h>

#include <spdlog/spdlog.h>

#include <queue>

namespace mwetris::network {

	TcpServer::TcpServer(asio::io_context& ioContext, const Settings& settings)
		: ServerCore(ioContext)
		, settings_{settings} {
	}

	TcpServer::~TcpServer() {
		spdlog::debug("[TcpServer] Destructor");
	}

	asio::awaitable<void> TcpServer::run() {
		co_await run(shared_from_this());
	}

	asio::awaitable<void> TcpServer::run(std::shared_ptr<TcpServer> server) {
		asio::ip::tcp::acceptor acceptor{server->ioContext_, server->getEndpoint()};
		while (!server->isStopped_) try {
			asio::ip::tcp::socket socket = co_await acceptor.async_accept(asio::use_awaitable);
			spdlog::info("[TcpServer] Accepted connection from {}", socket.remote_endpoint());

			server->spawnCoroutine(std::move(socket));
		} catch (const std::exception& e) {
			spdlog::error("[TcpServer] Exception: {}", e.what());
		}
		spdlog::debug("[TcpServer] Stopped");
	}

	void TcpServer::spawnCoroutine(asio::ip::tcp::socket socket) {
		auto remote = remotes_.emplace_back(Remote{
			.client = TcpClient::useExistingSocket(ioContext_, std::move(socket)),
			.clientId = ClientId::generateUniqueId()
		});
		asio::co_spawn(ioContext_, receivedFromClient(remote), asio::detached);
	}

	asio::ip::tcp::endpoint TcpServer::getEndpoint() const {
		return asio::ip::tcp::endpoint(asio::ip::tcp::v4(), static_cast<asio::ip::port_type>(settings_.port));
	}

}
