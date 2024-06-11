#ifndef MWETRIS_NETWORK_TCPSERVER_H
#define MWETRIS_NETWORK_TCPSERVER_H

#include "servercore.h"
#include "protobufmessage.h"
#include "protobufmessagequeue.h"
#include "client.h"
#include "game/playerslot.h"
#include "debugserver.h"
#include "tcpclient.h"
#include "asio.h"

#include "game/tetrisgame.h"


#include <mw/signal.h>

#include <memory>
#include <string>
#include <queue>

namespace mwetris::network {

	struct Session {
		asio::ip::tcp::socket socket;
		bool shutDown = false;
	};

	class TcpServer : public ServerCore, public std::enable_shared_from_this<TcpServer> {
	public:
		struct Settings {
			int port;
		};

		TcpServer(asio::io_context& ioContext, const Settings& settings);

		~TcpServer() override;

		asio::awaitable<void> run() override;

	private:
		static asio::awaitable<void> run(std::shared_ptr<TcpServer> server);

		void spawnCoroutine(asio::ip::tcp::socket socket);

		asio::ip::tcp::endpoint getEndpoint() const;

		Settings settings_;
	};

}

#endif
