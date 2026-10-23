#ifndef APP_UI_CONTROLSPRESENTER_H
#define APP_UI_CONTROLSPRESENTER_H

#include "../controlprofiles.h"
#include "../game/device.h"

#include <memory>
#include <string>
#include <vector>

namespace app {

	namespace game {

		class DeviceManager;
		class Player;

	}

}

namespace app::ui {

	class ControlsPresenter {
	public:
		ControlsPresenter(std::string windowName, std::shared_ptr<game::DeviceManager> deviceManager, std::shared_ptr<ControlProfiles> controlProfiles);

		void drawMenu();
		void drawProfileDialog();
		void drawBindingDialog();
		void drawDeviceProfileSelection(const std::shared_ptr<game::Player>& player, float width);

	private:
		enum class ProfileDialogAction {
			None,
			Duplicate,
			Edit,
			Delete,
		};

		struct DialogState {
			ProfileDialogAction action = ProfileDialogAction::None;
			ProfileId profileId;
			std::string deviceGuid;
			std::string profileName;
			std::string name;
			InputTiming timing;
			std::string error;
			std::weak_ptr<game::Player> player;
			bool openRequested = false;
		};

		struct BindingDialogState {
			game::DevicePtr device;
			std::string error;
			bool openRequested = false;
		};

		static const char* dialogTitle(ProfileDialogAction action);
		
		std::vector<game::DevicePtr> getConnectedDevices() const;
		game::DevicePtr resolveDevice(const std::string& deviceGuid, const std::vector<game::DevicePtr>& connectedDevices, bool retainDisconnected) const;
		
		bool drawDeviceCombo(std::string& selectedDeviceGuid, const std::vector<game::DevicePtr>& connectedDevices, 
			const char* label, bool retainDisconnected) const;
		
		void drawAssignedProfileCombo(const std::string& deviceGuid, const char* label, std::string& error) const;
		void drawPlayerProfileCombo(game::Player& player, const char* label) const;
		
		void openProfileDialog(ProfileDialogAction action, const InputProfile& profile,
			std::string deviceGuid = {}, std::weak_ptr<game::Player> player = {});
		void closeProfileDialog();
		void openBindingDialog(const game::DevicePtr& device);
		void closeBindingDialog();

		std::string selectedOptionsDeviceGuid_;
		std::string optionsError_;
		DialogState dialog_;
		BindingDialogState bindingDialog_;
		std::string popupIdSuffix_;
		std::string bindingPopupIdSuffix_;
		std::shared_ptr<game::DeviceManager> deviceManager_;
		std::shared_ptr<ControlProfiles> controlProfiles_;
	};

}

#endif
