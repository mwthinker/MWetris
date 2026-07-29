#ifndef APP_UI_CONTROLSPRESENTER_H
#define APP_UI_CONTROLSPRESENTER_H

#include <memory>
#include <string>

namespace app {

	class ControlProfiles;

	namespace game {

		class DeviceManager;
		class Player;

	}

}

namespace app::ui {

	class ControlsPresenter {
	public:
		ControlsPresenter(
			std::string windowName,
			std::shared_ptr<game::DeviceManager> deviceManager,
			std::shared_ptr<ControlProfiles> controlProfiles);
		~ControlsPresenter();

		ControlsPresenter(const ControlsPresenter&) = delete;
		ControlsPresenter& operator=(const ControlsPresenter&) = delete;

		void drawMenu();
		void drawProfileDialog();
		void drawDeviceProfileSelection(const std::shared_ptr<game::Player>& player, float width);

	private:
		struct Impl;
		std::unique_ptr<Impl> impl_;
	};

}

#endif
