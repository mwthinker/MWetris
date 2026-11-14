#ifndef APP_GAME_DEVICE_H
#define APP_GAME_DEVICE_H

#include "input.h"
#include "inputbindings.h"

#include <mw/signal.h>
#include <sdl/color.h>

#include <memory>
#include <string>
#include <utility>

namespace app::game {

	class Device;
	using DevicePtr = std::shared_ptr<Device>;

	class Device {
	public:
		friend class DeviceManager;

		Device(std::string name, std::string guid, DeviceKind kind, sdl::Color color)
			: name_{std::move(name)}
			, guid_{std::move(guid)}
			, kind_{kind}
			, color_{color} {
		}

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

		DeviceKind getKind() const {
			return kind_;
		}

		const sdl::Color& getColor() const {
			return color_;
		}

		/// @brief Check whether the physical device is connected to the computer.
		/// @return True when the device is connected; otherwise false.
		bool isConnected() const {
			return connected_;
		}

	private:
		void apply(const InputEvent& event) {
			const bool pressed = event.state == InputState::Pressed;
			switch (event.action) {
				case InputAction::Left: input_.left = pressed; break;
				case InputAction::Right: input_.right = pressed; break;
				case InputAction::SoftDrop: input_.down = pressed; break;
				case InputAction::Rotate: input_.rotate = pressed; break;
				case InputAction::HardDrop: input_.downGround = pressed; break;
			}
			if (pressed) {
				command.invoke(InputCommand{event.action, event.timestamp});
			}
		}

		void invokeConnection(bool connected) {
			connected_ = connected;
			connection.invoke(connected);
		}

		void invokeInputReset() {
			inputReset.invoke();
		}

		Input input_{};
		bool connected_ = false;
		std::string name_;
		std::string guid_;
		DeviceKind kind_;
		sdl::Color color_;
	};

}

#endif
