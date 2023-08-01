#ifndef MWETRIS_NETWORK_NETWORK_H
#define MWETRIS_NETWORK_NETWORK_H

#include <thread>
#include <memory>

#include <string>

namespace mwetris::network {

	class Network {
	public:
		Network();

		void connect();

		std::string getServerId() const {
			return {};
		}

	private:
		void run();

		class Impl;
		std::unique_ptr<Impl> impl_;
		std::jthread thread_;
	};

}

#endif
