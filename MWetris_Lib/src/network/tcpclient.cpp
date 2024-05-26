#include "tcpclient.h"
#include "protobufmessagequeue.h"
#include "util.h"
#include "game/player.h"
#include "debugserver.h"

#include <helper.h>

#include <spdlog/spdlog.h>

#include <queue>
#include <spdlog/spdlog.h>

namespace mwetris::network {

	namespace {
		
		constexpr int MaxSize = 1024;
		constexpr int Port = 12556;
		const std::string LocalHost = "127.0.0.1";

	}

	TcpClient::TcpClient(asio::io_context& ioContext)
		: ioContext_{ioContext}
		, timer_{ioContext}
		, socket_{ioContext}
		, name_{"TcpClient_Network"} {

		asio::co_spawn(ioContext_, [&]() -> asio::awaitable<void> {
			// TODO! Catch exception if something goes wrong?
			
			while (true) {
				try {
					co_await socket_.async_connect(asio::ip::tcp::endpoint(asio::ip::make_address_v4(LocalHost), Port), asio::use_awaitable);
					spdlog::error("[TcpClient] {} async_connect success", name_);
					break;
				} catch (std::exception& e) {
					spdlog::error("[TcpClient] {} async_connect Exception: {}", name_, e.what());
					// TODO! Make exponential backoff?
					timer_.expires_after(std::chrono::seconds{3});
				}
				co_await timer_.async_wait(asio::use_awaitable);
			}
		}, asio::detached);
	}

	TcpClient::TcpClient(asio::io_context& ioContext, asio::ip::tcp::socket socket)
		: ioContext_{ioContext}
		, timer_{ioContext}
		, socket_{std::move(socket)}
		, name_{"TcpClient_TcpServer"} {

		spdlog::warn("[TcpClient] {} 0 Socket is open: {}", name_, socket_.is_open());
	}

	TcpClient::~TcpClient() = default;

	asio::awaitable<ProtobufMessage> TcpClient::receive() {
		spdlog::warn("[TcpClient] {} 1 Socket is open: {}", name_, socket_.is_open());
		// TODO! Catch exception if something goes wrong?
		try {
			co_await socket_.async_wait(asio::ip::tcp::socket::wait_read, asio::use_awaitable);
		} catch (std::exception& e) {
			spdlog::error("[TcpClient] {} async_wait Exception: {}", name_, e.what());
			throw;
		}
		
		try {
			ProtobufMessage protobufMessage;
			queue_.acquire(protobufMessage);
			protobufMessage.clear();

			spdlog::warn("[TcpClient] {} 2 Socket is open: {}", name_, socket_.is_open());

			// Read header.
			std::size_t size = co_await socket_.async_read_some(protobufMessage.getMutableDataBuffer(), asio::use_awaitable);
			protobufMessage.reserveBodySize();

			// Read body.
			size = co_await socket_.async_read_some(protobufMessage.getMutableBodyBuffer(), asio::use_awaitable);

			co_return protobufMessage;
		} catch (std::exception& e) {
			spdlog::error("[TcpClient] {} 3 async_read_some Exception: {}", name_, e.what());
			throw;
		}
	}

	void TcpClient::send(ProtobufMessage&& message) {
		spdlog::warn("[TcpClient] {} Socket is open: {}", name_, socket_.is_open());
		auto buffer = message.getMutableDataBuffer();
		asio::async_write(socket_, buffer, asio::transfer_exactly(buffer.size()),
			// message is saved in pb so buffer is not destroyed.
			[this, pb = std::move(message)](std::error_code ec, std::size_t length) mutable {

			// TODO! Handle error.
			spdlog::warn("[TcpClient] {} 4 async_write Error code: {}, length: {}", name_, ec.message(), length);
			
			queue_.release(std::move(pb));
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

}
