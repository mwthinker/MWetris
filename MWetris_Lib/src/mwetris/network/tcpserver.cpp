#include "tcpserver.h"
#include "protobufmessagequeue.h"
#include "util.h"

#include "game/player.h"

#include <tetris/helper.h>

#include <spdlog/spdlog.h>

#include <queue>

#include <asio/error.hpp>

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
		ClientId clientId = ClientId::generateUniqueId();

		auto remote = Remote{
			.client = TcpClient::useExistingSocket(ioContext_, std::move(socket)),
			.clientId = clientId
		};
		remoteByClientId_[clientId] = remote;
		asio::co_spawn(ioContext_, handleClientSession(shared_from_this(), std::move(remote)), asio::detached);
	}

	asio::awaitable<void> TcpServer::handleClientSession(std::shared_ptr<TcpServer> server, Remote remote) {
		try {
			co_await server->receivedFromClient(remote);
		} catch (const std::system_error& e) {
			if (e.code() == asio::error::eof) {
				spdlog::debug("[TcpServer] handleClientSession {} : {}", e.code().message(), e.what());

				auto gameRoomOptional = server->findGameRoom(remote.clientId);
				if (gameRoomOptional.has_value()) {
					auto& gameRoom = gameRoomOptional.value().get();
					const auto& clientUuids = gameRoom.getConnectedClientUuids();
					if (clientUuids.size() < 2) {
						server->gameRoomById_.erase(gameRoom.getGameRoomId());
						server->roomIdByClientId_.erase(remote.clientId);
						server->remoteByClientId_.erase(remote.clientId);
						spdlog::info("[TcpServer] Game room {} closed due to last client disconnected", gameRoom.getGameRoomId());
					} else {
						// TODO! Wait for reconnection?
						spdlog::error("[TcpServer] Client disconnected from game room, waiting for reconnection");
					}
				}
			} else {
				spdlog::error("[TcpServer] System error: {}", e.what());
			}
		}
	}

	asio::ip::tcp::endpoint TcpServer::getEndpoint() const {
		return asio::ip::tcp::endpoint(asio::ip::tcp::v4(), static_cast<asio::ip::port_type>(settings_.port));
	}

}
