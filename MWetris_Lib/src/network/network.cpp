#include "network.h"

#include <message.pb.h>

#include <net/client.h>

#include <fmt/core.h>
#include <spdlog/spdlog.h>

namespace tp = tetris_protocol;

namespace mwetris::network {

	namespace {

		constexpr std::string_view ServerIp = "127.0.0.1";
		constexpr int Port = 59412;

	}

	Network::Network()
		: impl_{std::make_unique<Network::Impl>()} {

		//thread_ = std::jthread(&Network::run, this);
	}

	void Network::connect() {

	}

	void Network::run() {
		net::IoContext ioContext;

		auto client = net::Client::create(ioContext);
		bool connected = false;
		client->setReceiveHandler<tp::Wrapper>([](const tp::Wrapper& wrapper, const std::error_code& ec) {
			if (ec) {
				fmt::print("{}\n", ec.message());
			}
			wrapper.create_server_game();
			//fmt::print("Received: {}\n", message.text());
		});
		client->setDisconnectHandler([&](std::error_code ec) {
			connected = false;
			fmt::print("Disconnected: {}\n", ec.message());
		});
		client->setConnectHandler([&](std::error_code ec) {
			if (ec) {
				fmt::print("{}\n", ec.message());
				connected = false;
			} else {
				fmt::print("Connected\n");
				connected = true;
			}
		});

	}

	class Network::Impl {
	public:
		Impl() {
			


		}


	private:
		
	};


}