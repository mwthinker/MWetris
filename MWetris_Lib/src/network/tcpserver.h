#ifndef MWETRIS_NETWORK_TCPSERVER_H
#define MWETRIS_NETWORK_TCPSERVER_H

#include "servercore.h"
#include "protobufmessage.h"
#include "protobufmessagequeue.h"
#include "client.h"
#include "game/playerslot.h"
#include "debugserver.h"

#include "game/tetrisgame.h"

#include <asio.hpp>
#include <mw/signal.h>

#include <memory>
#include <string>
#include <queue>

namespace mwetris::network {

	class TcpServer : public ServerCore {
	public:
		explicit TcpServer(asio::io_context& ioContext)
			: ServerCore(ioContext) {
		}

		~TcpServer() override {
		}

		asio::awaitable<void> run() override {
			asio::ip::tcp::acceptor acceptor(ioContext_, {asio::ip::tcp::v4(), 12556});
			for (;;) try {
				asio::ip::tcp::socket socket = co_await acceptor.async_accept(asio::use_awaitable);
				spdlog::error("[TcpServer] Socket open: {}", socket.is_open());
				auto remote = remotes_.emplace_back(Remote{
					.client = std::make_shared<TcpClient>(ioContext_, std::move(socket)),
					.clientId = ClientId::generateUniqueId()
				});
				asio::co_spawn(ioContext_, receivedFromClient(remote), asio::detached);
			} catch (std::exception& e) {
				spdlog::error("[TcpServer] Exception: {}", e.what());
			}
		}

	};

}

#endif
