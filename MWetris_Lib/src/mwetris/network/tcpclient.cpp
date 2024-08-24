#include "tcpclient.h"
#include "protobufmessagequeue.h"

#include "game/player.h"
#include "util/auxiliary.h"

#include <tetris/helper.h>
#include <spdlog/spdlog.h>

#include <queue>
#include <spdlog/spdlog.h>

using namespace std::chrono_literals;

namespace mwetris::network {

	std::shared_ptr<TcpClient> TcpClient::connectToServer(asio::io_context& ioContext, const std::string& ip, int port) {
		auto client = std::shared_ptr<TcpClient>{new TcpClient{ioContext, ip, port}};

		spawnCoroutine(client);
		return client;
	}

	std::shared_ptr<TcpClient> TcpClient::useExistingSocket(asio::io_context& ioContext, asio::ip::tcp::socket socket) {
		return std::shared_ptr<TcpClient>{new TcpClient{ioContext, std::move(socket)}};
	}

	void TcpClient::stop() {
		spdlog::debug("[TcpClient] {} Stop", name_);
		try {
			socket_.close();
		} catch (const asio::system_error& e) {
			spdlog::error("[TcpClient] {} Stop Exception: {}", name_, e.what());
		}
		isStopped_ = true;
		connected_ = false;
	}

	bool TcpClient::isConnected() const {
		return connected_;
	}

	void TcpClient::reconnect() {
		if (endpoint_.address().is_unspecified()) {
			spdlog::error("[TcpClient] {} Can't reconnect, no endpoint.", name_);
			return;
		}

		try {
			spawnCoroutine(shared_from_this());
		} catch (const asio::system_error& e) {
			spdlog::error("[TcpClient] {} Reconnect Exception: {}", name_, e.what());
		}
	}

	asio::ip::tcp::endpoint TcpClient::getEndpoint() const {
		return endpoint_;
	}

	asio::awaitable<void> TcpClient::connect() {
		socket_ = asio::ip::tcp::socket{ioContext_};
		isStopped_ = false;
		connected_ = false;

		while (!connected_ && !isStopped_) {
			try {
				co_await socket_.async_connect(endpoint_, asio::use_awaitable);
				spdlog::debug("[TcpClient] {} async_connect success", name_);
				connected_ = true;
				break;
			} catch (const asio::system_error& e) {
				spdlog::error("[TcpClient] {} async_connect Exception: {}", name_, e.what());
				// TODO! Make exponential backoff?
				tryToConnectTimer_.expires_after(3s);
				connected_ = false;
			}
			co_await tryToConnectTimer_.async_wait(asio::use_awaitable);
		}
	}

	void TcpClient::spawnCoroutine(std::shared_ptr<TcpClient> client) {
		if (client->connected_) {
			spdlog::debug("[TcpClient] Can't reconnect, already connected.");
			return;
		}
		if (!client->isStopped_) {
			spdlog::debug("[TcpClient] Can't reconnect, must be stopped first.");
			return;
		}

		asio::co_spawn(client->getIoContext(), [client]() -> asio::awaitable<void> {
			auto coClient = client; // To keep shared_ptr alive in coroutine.

			co_await coClient->connect();
		}, asio::detached);
	}

	TcpClient::TcpClient(asio::io_context& ioContext, const std::string& ip, int port)
		: ioContext_{ioContext}
		, tryToConnectTimer_{ioContext}
		, waitingToConnect_{ioContext}
		, socket_{ioContext}
		, name_{"TcpClient_Network"} {
	
		assert(port > 0 && port < 65536);
		endpoint_ = asio::ip::tcp::endpoint{asio::ip::make_address_v4(ip), static_cast<asio::ip::port_type>(port)};	
	}

	TcpClient::TcpClient(asio::io_context& ioContext, asio::ip::tcp::socket socket)
		: ioContext_{ioContext}
		, tryToConnectTimer_{ioContext}
		, waitingToConnect_{ioContext}
		, socket_{std::move(socket)}
		, name_{"TcpClient_TcpServer"} {

		connected_ = true;
		spdlog::debug("[TcpClient] {} Created and socket is {}.", name_, socket_.is_open() ? "open" : "closed");
	}

	TcpClient::~TcpClient() {
		spdlog::debug("[TcpClient] {} Destroyed", name_);
	}

	asio::awaitable<ProtobufMessage> TcpClient::receive() {
		auto message = co_await receive(shared_from_this());
		co_return message;
	}

	asio::awaitable<ProtobufMessage> TcpClient::receive(std::shared_ptr<TcpClient> client) {
		co_await client->waitForConnection();

		ProtobufMessage message = co_await client->asyncRead();
		co_return message;
	}

	asio::awaitable<ProtobufMessage> TcpClient::asyncRead() {
		if (!connected_) {
			throw std::runtime_error("Not connected");
		}

		ProtobufMessage protobufMessage;
		queue_.acquire(protobufMessage);

		try {
			// Read header.
			protobufMessage.reserveHeaderSize();
			std::size_t size = co_await socket_.async_read_some(protobufMessage.getMutableDataBuffer(), asio::use_awaitable);

			// Read body.
			protobufMessage.reserveBodySize();
			size = co_await socket_.async_read_some(protobufMessage.getMutableBodyBuffer(), asio::use_awaitable);
		} catch (const asio::system_error& e) {
			spdlog::error("[TcpClient] {} async_read Exception: {}", name_, e.what());
			// TODO! When to stop?
			stop();
			throw e;
		}

		co_return protobufMessage;
	}

	asio::awaitable<void> TcpClient::waitForConnection() {
		if (connected_) {
			co_return;
		}

		waitingToConnect_.expires_after(0s);
		while (!connected_) {
			co_await waitingToConnect_.async_wait(asio::use_awaitable);
			waitingToConnect_.expires_after(0s);
		}
	}

	void TcpClient::send(ProtobufMessage&& message) {
		const auto buffer = message.getDataBuffer();
		
		asio::async_write(socket_, buffer, asio::transfer_exactly(buffer.size()),
			// message is saved in pb so buffer is not destroyed.
			[client = shared_from_this(), pb = std::move(message)](std::error_code ec, std::size_t length) mutable {

			if (ec) {
				spdlog::warn("[TcpClient] {} async_write Error code: {}, length: {}", client->name_, ec.message(), length);
			}
			
			client->release(std::move(pb));
		});
	}

	void TcpClient::acquire(ProtobufMessage& message) {
		queue_.acquire(message);
	}

	void TcpClient::release(ProtobufMessage&& message) {
		queue_.release(std::move(message));
	}

	asio::io_context& TcpClient::getIoContext() {
		return ioContext_;
	}

	const std::string& TcpClient::getName() const {
		return name_;
	}

}
