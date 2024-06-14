#ifndef MWETRIS_GAME_DEVICE_H
#define MWETRIS_GAME_DEVICE_H

#include "input.h"

#include <mw/signal.h>

#include <memory>
#include <string>

namespace mwetris::game {

	class Device;
	using DevicePtr = std::shared_ptr<Device>;

	class Device {
	public:
		friend class DeviceManager;

		mw::PublicSignal<Device, bool> connection;

		Input getInput() const {
			return input_;
		}

		const std::string& getName() const {
			return name_;
		}

		const std::string& getGuid() const {
			return guid_;
		}

		bool isConnected() const {
			return connected_;
		}
	
	private:
		void invokeConnection(bool connected) {
			connected_ = connected;
			connection.invoke(connected);
		}

		Input input_{};
		bool connected_ = false;
		std::string name_;
		std::string guid_;
	};

}

#endif
