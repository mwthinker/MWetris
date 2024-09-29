#include <protocol/server_to_client.pb.h>

#include <functional>
#include <string>
#include <fmt/format.h>

#include <argparse/argparse.hpp>
#include <fmt/printf.h>
#include <fmt/ostream.h>

using namespace std::chrono_literals;

template <> struct fmt::formatter<argparse::ArgumentParser> : fmt::ostream_formatter {};

/*
constexpr int Port = 5013;
const std::string LocalHost = "127.0.0.1";

constexpr int LanPort = 32012;

template <std::invocable T>
void repeatTimer(net::Timer& timer, std::chrono::seconds interval, T callback) {
	timer.expiresAfter(interval);
	timer.asyncWait([&timer, interval, callback](const std::error_code& error) {
		if (error) {
			fmt::print("repeatTimer {}\n", error.message());
			return;
		}
		callback();
		::repeatTimer<T>(timer, interval, callback);
	});
}

void runServer() {
	fmt::print("Start server\n");
	net::IoContext ioContext;

	auto server = net::Server::create(ioContext);

	server->setConnectHandler([&](const net::RemoteClientPtr& remoteClientPtr) {
		fmt::print("New Connection\n");

		remoteClientPtr->setReceiveHandler<tp::Game>([](const tp::Game& wrapper, const std::error_code& ec) {
			if (ec) {
				fmt::print("{}\n", ec.message());
			}

			//fmt::print("Received: {}\n", wrapper.text());
		});

		remoteClientPtr->setDisconnectHandler([](const std::error_code& ec) {
			if (ec) {
				fmt::print("{}\n", ec.message());
			}

			fmt::print("Disconnected\n");
		});
	});

	server->connect(Port);
	
	net::Timer timer{ioContext};
	int timerNbr = 0;
	repeatTimer(timer, 2s, [&timerNbr, &server]() {
		auto text = fmt::format("Server DATA {}\n", ++timerNbr);
		fmt::print("Send {}\n", text);
		//message::Wrapper wrapper;
		//wrapper.set_text(text);
		//server->sendToAll(wrapper);
	});

	ioContext.run();
}

void runClient() {
	fmt::print("Start client\n");

	net::IoContext ioContext;

	auto client = net::Client::create(ioContext);
	bool connected = false;
	client->setReceiveHandler<tp::Game>([](const tp::Game& message, const std::error_code& ec) {
		if (ec) {
			fmt::print("{}\n", ec.message());
		}

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

	client->connect(LocalHost, Port);
	
	net::Timer timer{ioContext};
	int timerNbr = 0;
	repeatTimer(timer, 3s, [&timerNbr, &client]() {
		auto text = fmt::format("Client DATA {}\n", ++timerNbr);
		//message::Wrapper wrapper;
		//wrapper.set_text(text);
		//client->send(wrapper);
	});

	ioContext.run();
}

void runServerLan() {
	fmt::print("Start server LAN\n");

	net::IoContext ioContext;
	net::LanUdpSender lanUdpSender{ioContext};

	bool disconnected = false;

	lanUdpSender.setDisconnectHandler([&](const std::error_code& ec) {
		if (ec) {
			fmt::print("{}\n", ec.message());
		}

		disconnected = true;
		fmt::print("Disconnected: {}\n", ec.message());
	});

	//message::Wrapper wrapper;
	//wrapper.set_text("hello");
	//lanUdpSender.setMessage(wrapper);
	
	lanUdpSender.connect(LanPort);

	while (!disconnected) {
		ioContext.runOne();
	}
}

void runClientLan() {
	fmt::print("Start client LAN\n");

	net::IoContext ioContext;
	net::LanUdpReceiver lanUdpReceiver{ioContext};

	lanUdpReceiver.setReceiveHandler<tp::Game>([](const net::Meta& meta, const tp::Game& wrapper, std::error_code ec) {
		if (ec) {
			fmt::print("{}\n", ec.message());
		}
		
		//fmt::print("{} | {}\n", meta.endpoint.address().to_string(), meta.endpoint.port());
		//fmt::print("Message: {}\n", wrapper.text());
	});

	lanUdpReceiver.connect(LanPort);
	ioContext.run();
}

*/
#include <network/tcpserver.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include <filesystem>

namespace {

	const std::string LogFile = "logs/mwetrisServer.log";

	void createLogFolder(const std::string& folderPath) {
		std::string logPath = "logs";
		if (!folderPath.empty()) {
			logPath = folderPath + "/logs";
		}
		try {
			if (std::filesystem::create_directory(logPath)) {
				fmt::print("Log folder created: {}\n", logPath);
			}
		} catch (const std::filesystem::filesystem_error& ex) {
			fmt::print("Log folder creation failed: {}\n", ex.what());
		}
	}

	bool initLog(const std::string& folderPath = "") {
		try {
			createLogFolder(folderPath);
			auto rotatingSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(folderPath + LogFile, 1024 * 1024, 10);
			auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

			auto logger = std::make_shared<spdlog::logger>("tetris", spdlog::sinks_init_list({consoleSink, rotatingSink}));
			logger->set_level(spdlog::level::trace);
			spdlog::set_default_logger(logger);
			return true;
		} catch (const spdlog::spdlog_ex& ex) {
			fmt::print("Log initialization failed: {}\n", ex.what());
			return false;
		}
	}

}

void runServer(int port) {
	initLog();
	spdlog::info("Start server");

	asio::io_context ioContext;

	auto settings = network::TcpServer::Settings{
		.port = port
	};

	auto server = std::make_shared<network::TcpServer>(ioContext, settings);
	server->start();
	ioContext.run();
}

int main(int argc, const char* argv[]) {
	int port = 11175;

	argparse::ArgumentParser program{"MWetrisServer", PROJECT_VERSION};
	program.add_description("Server for MWetris.");
	program.add_argument("-p", "--port")
		.help("tcp/ip port to receive connections from")
		.default_value(port)
		.scan<'i', int>();

	try {
		program.parse_args(argc, argv);
	} catch (const std::exception& err) {
		fmt::println("Error: {}", err.what());
		fmt::println("{}", program);
		return 1;
	}

	port = program.get<int>("-p");

	runServer(port);

	return 0;
}
