#include "tcpclient.h"
#include "protobufmessagequeue.h"
#include "util.h"

#include "game/player.h"

#include <tetris/helper.h>

#include <spdlog/spdlog.h>

#include <queue>
#include <spdlog/spdlog.h>

namespace mwetris::network {

	std::shared_ptr<TcpClient> TcpClient::connectToServer(asio::io_context& ioContext, const std::string& ip, int port) {
		assert(port > 0 && port < 65536);
		auto client = std::shared_ptr<TcpClient>{new TcpClient{ioContext, ip, port}};

		asio::co_spawn(ioContext, [tmp = client, ip = ip, port = port]() -> asio::awaitable<void> {
			auto client = tmp;
			// Must use ip and port before first co_await to guarantee lifetime.
			auto endpoint = asio::ip::tcp::endpoint{asio::ip::make_address_v4(ip), static_cast<asio::ip::port_type>(port)};

			while (!client->isStopped_) {
				try {
					co_await client->socket_.async_connect(endpoint, asio::use_awaitable);
					spdlog::debug("[TcpClient] {} async_connect success", client->name_);
					break;
				} catch (const std::exception& e) {
					spdlog::error("[TcpClient] {} async_connect Exception: {}", client->name_, e.what());
					// TODO! Make exponential backoff?
					client->timer_.expires_after(std::chrono::seconds{3});
				}
				co_await client->timer_.async_wait(asio::use_awaitable);
			}
		}, asio::detached);

		return client;
	}

	std::shared_ptr<TcpClient> TcpClient::useExistingSocket(asio::io_context& ioContext, asio::ip::tcp::socket socket) {
		return std::shared_ptr<TcpClient>{new TcpClient{ioContext, std::move(socket)}};
	}

	void TcpClient::stop() {
		spdlog::debug("[TcpClient] {} Stop", name_);
		try {
			socket_.close();
		} catch (const std::exception& e) {
			spdlog::error("[TcpClient] {} Stop Exception: {}", name_, e.what());
		}
		isStopped_ = true;
	}

	TcpClient::TcpClient(asio::io_context& ioContext, const std::string& ip, int port)
		: ioContext_{ioContext}
		, timer_{ioContext}
		, socket_{ioContext}
		, name_{"TcpClient_Network"} {
	}

	TcpClient::TcpClient(asio::io_context& ioContext, asio::ip::tcp::socket socket)
		: ioContext_{ioContext}
		, timer_{ioContext}
		, socket_{std::move(socket)}
		, name_{"TcpClient_TcpServer"} {

		spdlog::debug("[TcpClient] {} Created and socket is {}.", name_, socket_.is_open() ? "open" : "closed");
	}

	TcpClient::~TcpClient() {
		spdlog::debug("[TcpClient] {} Destroyed", name_);
	}

	asio::awaitable<ProtobufMessage> TcpClient::receive() {
		auto message = co_await receive(shared_from_this());
		co_return message;
	}

	asio::awaitable<ProtobufMessage> TcpClient::receive(std::shared_ptr<TcpClient> client) try {
		// Takes ownership of client.
		// TODO! Catch exception if something goes wrong?
		try {
			co_await client->socket_.async_wait(asio::ip::tcp::socket::wait_read, asio::use_awaitable);
		} catch (const std::exception& e) {
			spdlog::error("[TcpClient] {} async_wait Exception: {}", client->name_, e.what());
			throw;
		}

		ProtobufMessage message = co_await client->asyncRead();

		co_return message;
	} catch (const std::exception& e) {
		spdlog::error("[TcpClient] {} receive Exception: {}", client->name_, e.what());
		co_return ProtobufMessage{};
	}

	asio::awaitable<ProtobufMessage> TcpClient::asyncRead() try {
		ProtobufMessage protobufMessage;
		queue_.acquire(protobufMessage);

		// Read header.
		protobufMessage.reserveHeaderSize();
		std::size_t size = co_await socket_.async_read_some(protobufMessage.getMutableDataBuffer(), asio::use_awaitable);

		// Read body.
		protobufMessage.reserveBodySize();
		size = co_await socket_.async_read_some(protobufMessage.getMutableBodyBuffer(), asio::use_awaitable);

		co_return protobufMessage;
	} catch (const std::exception& e) {
		spdlog::error("[TcpClient] {} asyncRead Exception: {}", name_, e.what());
		co_return ProtobufMessage{};
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
