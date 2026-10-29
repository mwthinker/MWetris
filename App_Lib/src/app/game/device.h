#ifndef APP_GAME_DEVICE_H
#define APP_GAME_DEVICE_H

#include "input.h"
#include <mw/signal.h>

#include <memory>
#include <string>

namespace app::game {

	class Device;
	using DevicePtr = std::shared_ptr<Device>;

	class Device {
	public:
		friend class DeviceManager;

		mw::PublicSignal<Device, bool> connection;
		mw::PublicSignal<Device, const InputCommand&> command;
		mw::PublicSignal<Device> inputReset;

		Input getInput() const {
			return input_;
		}

		/// @brief Get the implementation-dependent name for a game controller.
		const std::string& getName() const {
			return name_;
		}

		/// @brief Return the GUID of the device. A stable unique ID attached to the hardware.
		/// @return The GUID of the device.
		const std::string& getGuid() const {
			return guid_;
		}

		/// @brief Check whether the physical device is connected to the computer.
		/// @return True when the device is connected; otherwise false.
		bool isConnected() const {
			return connected_;
		}

	private:
		void invokeConnection(bool connected) {
			connected_ = connected;
			connection.invoke(connected);
		}

		void invokeCommand(const InputCommand& inputCommand) {
			command.invoke(inputCommand);
		}

		void invokeInputReset() {
			inputReset.invoke();
		}

		Input input_{};
		bool connected_ = false;
		std::string name_;
		std::string guid_;
	};

}

#endif
