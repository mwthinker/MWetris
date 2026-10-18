#include "controlspresenter.h"

#include "imguiextra.h"

#include "controlprofiles.h"
#include "game/devicemanager.h"
#include "game/player.h"

#include <imgui.h>

#include <algorithm>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace app::ui {

	const char* ControlsPresenter::dialogTitle(ProfileDialogAction action) {
		switch (action) {
			case ProfileDialogAction::Duplicate:
				return "Duplicate profile";
			case ProfileDialogAction::Edit:
				return "Edit profile";
			case ProfileDialogAction::Delete:
				return "Delete profile";
			case ProfileDialogAction::None:
				return "Control profile";
		}
		return "Control profile";
	}

	std::vector<game::DevicePtr> ControlsPresenter::getConnectedDevices() const {
		return deviceManager_->getAllDevicesAvailable();
	}

	game::DevicePtr ControlsPresenter::resolveDevice(const std::string& deviceGuid, const std::vector<game::DevicePtr>& connectedDevices, 
		bool retainDisconnected) const {

		auto connected = std::ranges::find_if(connectedDevices, [&](const game::DevicePtr& device) {
			return device->getGuid() == deviceGuid;
		});
		if (connected != connectedDevices.end()) {
			return *connected;
		}
		return retainDisconnected ? deviceManager_->findDevice(deviceGuid) : nullptr;
	}

	bool ControlsPresenter::drawDeviceCombo(std::string& selectedDeviceGuid, const std::vector<game::DevicePtr>& connectedDevices,
		const char* label, bool retainDisconnected) const {

		bool changed = false;
		auto selectedDevice = resolveDevice(selectedDeviceGuid, connectedDevices, retainDisconnected);
		if (!selectedDevice && !connectedDevices.empty()) {
			selectedDevice = connectedDevices.front();
			changed = selectedDeviceGuid != selectedDevice->getGuid();
			selectedDeviceGuid = selectedDevice->getGuid();
		} else if (!selectedDevice && !retainDisconnected) {
			changed = !selectedDeviceGuid.empty();
			selectedDeviceGuid.clear();
		}

		std::string preview = "No connected devices";
		if (selectedDevice) {
			preview = selectedDevice->getName();
			if (!selectedDevice->isConnected()) {
				preview += " (disconnected)";
			}
		}

		ImGui::ComboScoped(label, preview.c_str(), [&]() {
			for (const auto& device : connectedDevices) {
				ImGui::PushID(device->getGuid().c_str());
				ImGui::PushID(device.get());
				bool isSelected = selectedDeviceGuid == device->getGuid();
				if (ImGui::Selectable(device->getName().c_str(), isSelected)) {
					selectedDeviceGuid = device->getGuid();
					changed = true;
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
				ImGui::PopID();
				ImGui::PopID();
			}
		});
		return changed;
	}

	void ControlsPresenter::drawAssignedProfileCombo(
		const std::string& deviceGuid,
		const char* label,
		std::string& error) const {
		if (deviceGuid.empty()) {
			ImGui::BeginDisabled();
			ImGui::ComboScoped(label, "No device", []() {});
			ImGui::EndDisabled();
			return;
		}

		auto selectedProfileId = controlProfiles_->getAssignedProfileId(deviceGuid);
		auto selectedProfile = controlProfiles_->getProfile(selectedProfileId);
		const auto profiles = controlProfiles_->getProfiles();
		std::optional<ProfileId> requestedProfileId;
		ImGui::ComboScoped(label, selectedProfile ? selectedProfile->name.c_str() : "Unknown profile", [&]() {
			for (const auto& profile : profiles) {
				ImGui::PushID(profile.id.value.c_str());
				bool isSelected = selectedProfileId == profile.id;
				if (ImGui::Selectable(profile.name.c_str(), isSelected)) {
					requestedProfileId = profile.id;
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
				ImGui::PopID();
			}
		});

		if (requestedProfileId) {
			auto result = controlProfiles_->assignProfile(deviceGuid, *requestedProfileId);
			error = result ? std::string{} : result.error().message;
		}
	}

	void ControlsPresenter::drawPlayerProfileCombo(game::Player& player, const char* label) const {
		auto selectedProfileId = player.getProfileId();
		if (!selectedProfileId) {
			ImGui::BeginDisabled();
			ImGui::ComboScoped(label, "No controls", []() {});
			ImGui::EndDisabled();
			return;
		}

		auto selectedProfile = controlProfiles_->getProfile(*selectedProfileId);
		const auto& profiles = controlProfiles_->getProfiles();
		std::optional<ProfileId> requestedProfileId;
		ImGui::ComboScoped(label, selectedProfile ? selectedProfile->name.c_str() : "Unknown profile", [&]() {
			for (const auto& profile : profiles) {
				ImGui::PushID(profile.id.value.c_str());
				bool isSelected = *selectedProfileId == profile.id;
				if (ImGui::Selectable(profile.name.c_str(), isSelected)) {
					requestedProfileId = profile.id;
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
				ImGui::PopID();
			}
		});
		if (requestedProfileId) {
			player.setProfileId(*requestedProfileId);
		}
	}

	void ControlsPresenter::openProfileDialog(
		ProfileDialogAction action,
		const InputProfile& profile,
		std::string deviceGuid) {
		dialog_ = {
			.action = action,
			.profileId = profile.id,
			.deviceGuid = std::move(deviceGuid),
			.profileName = profile.name,
			.name = profile.name,
			.timing = profile.timing,
			.openRequested = true,
		};
		if (action == ProfileDialogAction::Duplicate) {
			auto duplicateName = controlProfiles_->getDuplicateProfileName(profile.id);
			if (duplicateName) {
				dialog_.name = std::move(*duplicateName);
			} else {
				dialog_.error = duplicateName.error().message;
			}
		}
	}

	void ControlsPresenter::closeProfileDialog() {
		dialog_ = {};
		ImGui::CloseCurrentPopup();
	}

	ControlsPresenter::ControlsPresenter(
		std::string windowName,
		std::shared_ptr<game::DeviceManager> deviceManager,
		std::shared_ptr<ControlProfiles> controlProfiles)
		: popupIdSuffix_{"###ControlProfileDialog-" + std::move(windowName)}
		, deviceManager_{std::move(deviceManager)}
		, controlProfiles_{std::move(controlProfiles)} {
	}

	void ControlsPresenter::drawMenu() {
		auto connectedDevices = getConnectedDevices();
		ImGui::TextUnformatted("Controls");
		ImGui::Separator();

		ImGui::PushItemWidth(180.f);
		if (drawDeviceCombo(
			selectedOptionsDeviceGuid_,
			connectedDevices,
			"Device",
			false)) {
			optionsError_.clear();
		}
		drawAssignedProfileCombo(selectedOptionsDeviceGuid_, "Profile", optionsError_);
		ImGui::PopItemWidth();

		if (selectedOptionsDeviceGuid_.empty()) {
			if (!optionsError_.empty()) {
				ImGui::TextWrapped("%s", optionsError_.c_str());
			}
			return;
		}

		auto profileId = controlProfiles_->getAssignedProfileId(selectedOptionsDeviceGuid_);
		auto profile = controlProfiles_->getProfile(profileId);
		if (!profile) {
			ImGui::TextDisabled("The assigned profile is unavailable.");
			return;
		}
		ImGui::Text("DAS %d ms   ARR %d ms", profile->timing.dasMs, profile->timing.arrMs);

		if (ImGui::Button("Duplicate")) {
			openProfileDialog(
				ProfileDialogAction::Duplicate,
				*profile,
				selectedOptionsDeviceGuid_);
		}
		ImGui::SameLine();
		ImGui::BeginDisabled(profile->builtIn);
		if (ImGui::Button("Edit")) {
			openProfileDialog(ProfileDialogAction::Edit, *profile);
		}
		ImGui::SameLine();
		if (ImGui::Button("Delete")) {
			openProfileDialog(ProfileDialogAction::Delete, *profile);
		}
		ImGui::EndDisabled();
		if (profile->builtIn) {
			ImGui::TextDisabled("Built-in profiles can only be duplicated.");
		}
		if (!optionsError_.empty()) {
			ImGui::TextWrapped("%s", optionsError_.c_str());
		}
	}

	void ControlsPresenter::drawProfileDialog() {
		auto& dialog = dialog_;
		auto popupId = std::string{dialogTitle(dialog.action)} + popupIdSuffix_;
		if (dialog.openRequested) {
			dialog.openRequested = false;
			ImGui::OpenPopup(popupId.c_str());
			ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, {0.5f, 0.5f});
			ImGui::SetNextWindowSize({360.f, 0.f}, ImGuiCond_Appearing);
		}

		if (!ImGui::BeginPopupModal(popupId.c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
			return;
		}

		if (dialog.action == ProfileDialogAction::Delete) {
			ImGui::Text("Delete profile '%s'?", dialog.profileName.c_str());
			auto assignedDeviceCount = controlProfiles_->getAssignedDeviceCount(dialog.profileId);
			if (assignedDeviceCount == 0) {
				ImGui::TextDisabled("No devices use this profile.");
			} else if (assignedDeviceCount == 1) {
				ImGui::TextDisabled("1 device uses this profile and will switch to Balanced.");
			} else {
				ImGui::TextDisabled("%d devices use this profile and will switch to Balanced.", assignedDeviceCount);
			}

			if (!dialog.error.empty()) {
				ImGui::TextWrapped("%s", dialog.error.c_str());
			}
			if (ImGui::AbortButton("Delete")) {
				auto result = controlProfiles_->deleteProfile(dialog.profileId);
				if (result) {
					closeProfileDialog();
				} else {
					dialog.error = result.error().message;
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
				closeProfileDialog();
			}
			ImGui::EndPopup();
			return;
		}

		bool duplicate = dialog.action == ProfileDialogAction::Duplicate;
		if (duplicate && ImGui::IsWindowAppearing()) {
			ImGui::SetKeyboardFocusHere();
		}
		ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue;
		if (duplicate) {
			inputFlags |= ImGuiInputTextFlags_AutoSelectAll;
		}
		bool submitted = ImGui::InputText("Name", &dialog.name, inputFlags);
		bool changed = ImGui::IsItemEdited();
		ImGui::PushItemWidth(180.f);
		changed |= ImGui::SliderInt(
			"DAS",
			&dialog.timing.dasMs,
			ControlProfiles::MinDasMs,
			ControlProfiles::MaxDasMs,
			"%d ms",
			ImGuiSliderFlags_AlwaysClamp);
		changed |= ImGui::SliderInt(
			"ARR",
			&dialog.timing.arrMs,
			ControlProfiles::MinArrMs,
			ControlProfiles::MaxArrMs,
			"%d ms",
			ImGuiSliderFlags_AlwaysClamp);
		ImGui::PopItemWidth();
		if (changed) {
			dialog.error.clear();
		}

		if (!dialog.error.empty()) {
			ImGui::TextWrapped("%s", dialog.error.c_str());
		}

		const char* confirmationLabel = duplicate ? "Create" : "Save";
		if (ImGui::ConfirmationButton(confirmationLabel) || submitted) {
			if (duplicate) {
				auto result = controlProfiles_->duplicateProfile(
					dialog.deviceGuid,
					dialog.profileId,
					dialog.name,
					dialog.timing);
				if (result) {
					closeProfileDialog();
				} else {
					dialog.error = result.error().message;
				}
			} else {
				auto result = controlProfiles_->editProfile(
					dialog.profileId,
					dialog.name,
					dialog.timing);
				if (result) {
					closeProfileDialog();
				} else {
					dialog.error = result.error().message;
				}
			}
		}
		ImGui::SameLine();
		if (ImGui::AbortButton("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
			closeProfileDialog();
		}

		ImGui::EndPopup();
	}

	void ControlsPresenter::drawDeviceProfileSelection(const std::shared_ptr<game::Player>& player, float width) {
		auto playerDevice = player->getDevice();
		auto deviceGuid = playerDevice ? playerDevice->getGuid() : std::string{};

		ImGui::PushID(player.get());
		ImGui::TextUnformatted("Device");
		ImGui::PushItemWidth(width);
		auto connectedDevices = getConnectedDevices();
		if (drawDeviceCombo(deviceGuid, connectedDevices, "##Device", true)) {
			player->setDevice(deviceManager_->findDevice(deviceGuid));
		}
		ImGui::TextUnformatted("Profile");
		drawPlayerProfileCombo(*player, "##Profile");
		ImGui::PopItemWidth();
		ImGui::PopID();
	}

}
