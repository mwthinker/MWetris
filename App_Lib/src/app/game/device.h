#ifndef APP_GAME_DEVICE_H
#define APP_GAME_DEVICE_H

#include "input.h"
#include "../controlprofiles.h"

#include <mw/signal.h>

#include <cstdint>
#include <memory>
#include <string>

namespace app::game {

	class Device;
	using DevicePtr = std::shared_ptr<Device>;

	class Device {
	public:
		friend class DeviceManager;

		mw::PublicSignal<Device, bool> connection;

		Input getInput() const {
			return input_;
		}

		/// @brief Get the implementation-dependent name for an game controller.
		const std::string& getName() const {
			return name_;
		}

		/// @brief Return the GUID of the device. A stable unique id for the device, i.e. attached to the hardware.
		/// @return the GUID of the device.
		const std::string& getGuid() const {
			return guid_;
		}

		/// @brief Is the device connected? I.e. the physical device is connected to the computer.
		/// @return true if the device is connected else false.
		bool isConnected() const {
			return connected_;
		}

		app::InputTiming getInputTiming() const {
			return inputTiming_;
		}

		std::uint64_t getInputTimingRevision() const {
			return inputTimingRevision_;
		}
	
	private:
		void setInputTiming(app::InputTiming timing) {
			if (inputTiming_ == timing) {
				return;
			}
			inputTiming_ = timing;
			++inputTimingRevision_;
		}

		void invokeConnection(bool connected) {
			connected_ = connected;
			connection.invoke(connected);
		}

		Input input_{};
		bool connected_ = false;
		std::string name_;
		std::string guid_;
		app::InputTiming inputTiming_{};
		std::uint64_t inputTimingRevision_ = 0;
	};

}

#endif
