#ifndef APP_GAME_DEVICE_H
#define APP_GAME_DEVICE_H

#include "input.h"
#include "inputbindings.h"

#include <mw/signal.h>

#include <array>
#include <functional>
#include <memory>
#include <optional>
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

		void beginMapping() {
			if (beginMapping_) {
				beginMapping_();
			}
		}

		void resetMapping() {
			if (resetMapping_) {
				resetMapping_();
			}
		}

		void cancelMapping() {
			if (cancelMapping_) {
				cancelMapping_();
			}
		}

		std::optional<MappingStatus> getMappingStatus() const {
			return getMappingStatus_ ? getMappingStatus_() : std::nullopt;
		}

		std::array<std::string, 5> getMappingLabels() const {
			return getMappingLabels_ ? getMappingLabels_() : std::array<std::string, 5>{};
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
		std::function<void()> beginMapping_;
		std::function<void()> resetMapping_;
		std::function<void()> cancelMapping_;
		std::function<std::optional<MappingStatus>()> getMappingStatus_;
		std::function<std::array<std::string, 5>()> getMappingLabels_;
	};

}

#endif
