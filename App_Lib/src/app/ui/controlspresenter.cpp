#include "controlspresenter.h"

#include "imguiextra.h"
#include "theme.h"

#include "controlprofiles.h"
#include "game/devicemanager.h"
#include "game/player.h"

#include <imgui.h>
#include <IconsFontAwesome6.h>

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
		std::string deviceGuid,
		std::weak_ptr<game::Player> player) {
		dialog_ = {
			.action = action,
			.profileId = profile.id,
			.deviceGuid = std::move(deviceGuid),
			.profileName = profile.name,
			.name = profile.name,
			.timing = profile.timing,
			.player = std::move(player),
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

	void ControlsPresenter::openBindingDialog(const game::DevicePtr& device) {
		if (!device || !device->isConnected()) {
			return;
		}
		if (bindingDialog_.device) {
			deviceManager_->cancelMapping(*bindingDialog_.device);
		}
		bindingDialog_ = {
			.device = device,
			.openRequested = true,
		};
		deviceManager_->beginMapping(*device);
	}

	void ControlsPresenter::closeBindingDialog() {
		if (bindingDialog_.device) {
			deviceManager_->cancelMapping(*bindingDialog_.device);
		}
		bindingDialog_ = {};
		ImGui::CloseCurrentPopup();
	}

	ControlsPresenter::ControlsPresenter(
		std::string windowName,
		std::shared_ptr<game::DeviceManager> deviceManager,
		std::shared_ptr<ControlProfiles> controlProfiles)
		: popupIdSuffix_{"###ControlProfileDialog-" + windowName}
		, bindingPopupIdSuffix_{"###DeviceBindingDialog-" + std::move(windowName)}
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
			ImGui::SecondaryText("The assigned profile is unavailable.");
			return;
		}
		ImGui::Text("DAS %d ms   ARR %d ms", profile->timing.dasMs, profile->timing.arrMs);

		if (ImGui::MediumButton("Duplicate")) {
			openProfileDialog(
				ProfileDialogAction::Duplicate,
				*profile,
				selectedOptionsDeviceGuid_);
		}
		ImGui::SameLine();
		ImGui::BeginDisabled(profile->builtIn);
		if (ImGui::MediumButton("Edit")) {
			openProfileDialog(ProfileDialogAction::Edit, *profile);
		}
		ImGui::SameLine();
		if (ImGui::MediumButton("Delete")) {
			openProfileDialog(ProfileDialogAction::Delete, *profile);
		}
		ImGui::EndDisabled();
		if (profile->builtIn) {
			ImGui::SecondaryText("Built-in profiles can only be duplicated.");
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
		}
		const bool deleteDialog = dialog.action == ProfileDialogAction::Delete;
		ImGui::SetNextModalWindow(deleteDialog ? ImVec2{560.f, 260.f} : ImVec2{560.f, 440.f});
		ImGui::PushContentStyle();

		if (!ImGui::BeginPopupModal(popupId.c_str(), nullptr,
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar
				| ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
			ImGui::PopContentStyle();
			return;
		}

		ImGui::PushFont(Configuration::getInstance().getImGuiSemiBoldFont(), 0.f);
		ImGui::TextUnformatted(dialogTitle(dialog.action));
		ImGui::PopFont();
		ImGui::Separator();
		ImGui::BeginModalBody("Profile content");

		if (deleteDialog) {
			ImGui::Text("Delete profile '%s'?", dialog.profileName.c_str());
			auto assignedDeviceCount = controlProfiles_->getAssignedDeviceCount(dialog.profileId);
			if (assignedDeviceCount == 0) {
				ImGui::SecondaryText("No devices use this profile.");
			} else if (assignedDeviceCount == 1) {
				ImGui::SecondaryText("1 device uses this profile and will switch to Balanced.");
			} else {
				ImGui::SecondaryText("%d devices use this profile and will switch to Balanced.", assignedDeviceCount);
			}

			if (!dialog.error.empty()) {
				ImGui::TextColored(theme::ErrorText, "%s", dialog.error.c_str());
			}
			ImGui::EndModalBody();
			const float actionWidth = 180.f + ImGui::GetStyle().ItemSpacing.x;
			ImGui::AlignNextItemRight(actionWidth);
			if (ImGui::MediumButton("Cancel", {90.f, 0.f}) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
				closeProfileDialog();
			}
			ImGui::SameLine();
			if (ImGui::AbortButton("Delete", {90.f, 0.f})) {
				auto result = controlProfiles_->deleteProfile(dialog.profileId);
				if (result) {
					closeProfileDialog();
				} else {
					dialog.error = result.error().message;
				}
			}
			ImGui::EndPopup();
			ImGui::PopContentStyle();
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
		bool submitted = false;
		bool changed = false;
		ImGui::FormTable("Profile name form", [&]() {
			ImGui::FormRow("Name", [&]() {
				submitted = ImGui::InputText("##Value", &dialog.name, inputFlags);
				changed |= ImGui::IsItemEdited();
			});
		}, 120.f);

		ImGui::Dummy({0.f, 6.f});
		ImGui::PushFont(Configuration::getInstance().getImGuiSemiBoldFont(), 0.f);
		ImGui::SeparatorText("Repeat timing");
		ImGui::PopFont();
		ImGui::FormTable("Profile timing form", [&]() {
			ImGui::FormRow("DAS delay", [&]() {
				changed |= ImGui::SliderInt("##Value", &dialog.timing.dasMs,
					ControlProfiles::MinDasMs, ControlProfiles::MaxDasMs, "%d ms", ImGuiSliderFlags_AlwaysClamp);
			});
			ImGui::FormRow("ARR interval", [&]() {
				changed |= ImGui::SliderInt("##Value", &dialog.timing.arrMs,
					ControlProfiles::MinArrMs, ControlProfiles::MaxArrMs, "%d ms", ImGuiSliderFlags_AlwaysClamp);
			});
		}, 120.f);
		ImGui::SecondaryText("DAS is the delay before a held direction starts repeating.");
		ImGui::SecondaryText("ARR is the interval between repeated movements.");
		if (changed) {
			dialog.error.clear();
		}

		if (!dialog.error.empty()) {
			ImGui::TextColored(theme::ErrorText, "%s", dialog.error.c_str());
		}

		const char* confirmationLabel = duplicate ? "Create" : "Save";
		ImGui::EndModalBody();
		const float actionWidth = 180.f + ImGui::GetStyle().ItemSpacing.x;
		ImGui::AlignNextItemRight(actionWidth);
		if (ImGui::MediumButton("Cancel", {90.f, 0.f}) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
			closeProfileDialog();
		}
		ImGui::SameLine();
		if (ImGui::ConfirmationButton(confirmationLabel, {90.f, 0.f}) || submitted) {
			if (duplicate) {
				auto result = controlProfiles_->duplicateProfile(
					dialog.deviceGuid,
					dialog.profileId,
					dialog.name,
					dialog.timing);
				if (result) {
					if (auto player = dialog.player.lock()) {
						player->setProfileId(*result);
					}
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
		ImGui::EndPopup();
		ImGui::PopContentStyle();
	}

	void ControlsPresenter::drawBindingDialog() {
		auto& dialog = bindingDialog_;
		const auto popupId = std::string{"Configure controls"} + bindingPopupIdSuffix_;
		if (dialog.openRequested) {
			dialog.openRequested = false;
			ImGui::OpenPopup(popupId.c_str());
		}
		if (!dialog.device) {
			return;
		}

		auto status = deviceManager_->getMappingStatus(*dialog.device);
		if (!status) {
			ImGui::SetNextModalWindow({620.f, 600.f});
			if (ImGui::BeginPopupModal(popupId.c_str(), nullptr, ImGuiWindowFlags_NoTitleBar)) {
				ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}
			dialog = {};
			return;
		}
		const float scale = std::clamp(ImGui::GetFontSize() / 17.f, 0.5f, 2.f);
		ImGui::SetNextModalWindow({620.f, 600.f});
		ImGui::PushContentStyle();
		if (!ImGui::BeginPopupModal(popupId.c_str(), nullptr,
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar
				| ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
			ImGui::PopContentStyle();
			deviceManager_->cancelMapping(*dialog.device);
			dialog = {};
			return;
		}

		ImGui::PushFont(Configuration::getInstance().getImGuiSemiBoldFont(), 0.f);
		ImGui::Text("Configure %s", dialog.device->getName().c_str());
		ImGui::PopFont();
		ImGui::Separator();
		ImGui::Dummy({0.f, 4.f * scale});
		ImGui::SecondaryText("Mapping is active. Enter each control in order.");
		if (status->completed < status->total) {
			const char* inputName = status->acceptedInput == game::InputKind::KeyboardKey ? "key" : "gamepad button";
			ImGui::Text("Press a %s for %s", inputName, game::getInputActionName(status->awaiting));
		} else {
			ImGui::TextUnformatted("All actions captured. Review and save.");
		}
		ImGui::BeginModalBody("Binding content", true);
		auto labels = deviceManager_->getMappingLabels(*dialog.device);
		ImGui::FormTable("Input bindings", [&]() {
			for (int index = 0; index < std::ssize(game::InputActions); ++index) {
				const bool active = status->completed < status->total
					&& game::InputActions[index] == status->awaiting;
				const bool completed = index < status->completed;
				ImGui::TableNextRow(ImGuiTableRowFlags_None, ImGui::GetFrameHeight());
				if (active) {
					ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImGuiCol_HeaderHovered));
					ImGui::PushFont(Configuration::getInstance().getImGuiSemiBoldFont(), 0.f);
				} else if (completed) {
					ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
				}

				ImGui::TableNextColumn();
				ImGui::AlignTextToFramePadding();
				if (active) {
					ImGui::Text(">  %s", game::getInputActionName(game::InputActions[index]));
				} else if (completed) {
					ImGui::Text("[x]  %s", game::getInputActionName(game::InputActions[index]));
				} else {
					ImGui::TextUnformatted(game::getInputActionName(game::InputActions[index]));
				}
				ImGui::TableNextColumn();
				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted(active ? "Press now" : labels[index].c_str());

				if (active) {
					ImGui::PopFont();
				} else if (completed) {
					ImGui::PopStyleColor();
				}
			}
		}, 130.f * scale);

		if (!dialog.error.empty()) {
			ImGui::TextColored(theme::ErrorText, "%s", dialog.error.c_str());
		}
		ImGui::EndModalBody();
		if (ImGui::MediumButton("Start over", {110.f * scale, 0.f})) {
			deviceManager_->beginMapping(*dialog.device);
			dialog.error.clear();
		}
		ImGui::SameLine();
		if (ImGui::MediumButton("Defaults", {90.f * scale, 0.f})) {
			deviceManager_->resetMapping(*dialog.device);
			dialog.error.clear();
		}
		ImGui::SameLine();
		const float actionWidth = 190.f * scale + ImGui::GetStyle().ItemSpacing.x;
		const float rightStart = ImGui::GetWindowWidth() - ImGui::GetStyle().WindowPadding.x - actionWidth;
		ImGui::SetCursorPosX(std::max(ImGui::GetCursorPosX(), rightStart));
		if (ImGui::MediumButton("Cancel", {90.f * scale, 0.f})) {
			closeBindingDialog();
		} else {
			ImGui::SameLine();
			ImGui::BeginDisabled(status->completed < status->total);
			if (ImGui::ConfirmationButton("Save", {90.f * scale, 0.f})) {
				auto result = deviceManager_->saveMapping(*dialog.device);
				if (result) {
					dialog = {};
					ImGui::CloseCurrentPopup();
				} else {
					dialog.error = result.error();
				}
			}
			ImGui::EndDisabled();
		}
		ImGui::EndPopup();
		ImGui::PopContentStyle();
	}

	void ControlsPresenter::drawDeviceProfileSelection(const std::shared_ptr<game::Player>& player, float width) {
		auto playerDevice = player->getDevice();
		auto deviceGuid = playerDevice ? playerDevice->getGuid() : std::string{};
		auto& configuration = Configuration::getInstance();
		auto textColor = configuration.getPlayerCardTextColor().toImU32();
		auto controlColor = configuration.getPreviewBackgroundColor().toImU32();
		auto hoverColor = configuration.getSidebarBackgroundColor().toImU32();
		auto activeColor = configuration.getPlayerCardBackgroundColor().toImU32();
		auto borderColor = configuration.getPreviewBorderColor().toImU32();

		ImGui::PushID(player.get());
		ImGui::PushStyleColor(ImGuiCol_Text, textColor);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, controlColor);
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, hoverColor);
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, activeColor);
		ImGui::PushStyleColor(ImGuiCol_PopupBg, configuration.getSidebarBackgroundColor().toImU32());
		ImGui::PushStyleColor(ImGuiCol_Header, controlColor);
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, activeColor);
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, controlColor);
		ImGui::PushStyleColor(ImGuiCol_Border, borderColor);
		ImGui::PushStyleColor(ImGuiCol_Button, controlColor);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoverColor);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, activeColor);
		ImGui::PushFont(configuration.getImGuiRegularSmallFont(), 0.f);
		ImGui::TextUnformatted("Device");
		ImGui::PopFont();
		auto connectedDevices = getConnectedDevices();
		const float buttonWidth = ImGui::GetFrameHeight();
		const float comboWidth = std::max(1.f, width - buttonWidth - ImGui::GetStyle().ItemSpacing.x);
		ImGui::PushItemWidth(comboWidth);
		if (drawDeviceCombo(deviceGuid, connectedDevices, "##Device", true)) {
			player->setDevice(deviceManager_->findDevice(deviceGuid));
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();
		playerDevice = player->getDevice();
		ImGui::BeginDisabled(!playerDevice || !playerDevice->isConnected());
		if (ImGui::Button(ICON_FA_WRENCH "##ConfigureDevice", {buttonWidth, 0.f})) {
			openBindingDialog(playerDevice);
		}
		ImGui::EndDisabled();
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
			ImGui::SetTooltip("Configure controls");
		}
		ImGui::PushFont(configuration.getImGuiRegularSmallFont(), 0.f);
		ImGui::TextUnformatted("Profile");
		ImGui::PopFont();
		ImGui::PushItemWidth(comboWidth);
		drawPlayerProfileCombo(*player, "##Profile");
		ImGui::PopItemWidth();
		ImGui::SameLine();
		auto profileId = player->getProfileId();
		auto profile = profileId ? controlProfiles_->getProfile(*profileId) : std::nullopt;
		ImGui::BeginDisabled(!profile);
		if (ImGui::Button(ICON_FA_WRENCH "##ConfigureProfile", {buttonWidth, 0.f}) && profile) {
			openProfileDialog(
				profile->builtIn ? ProfileDialogAction::Duplicate : ProfileDialogAction::Edit,
				*profile,
				playerDevice ? playerDevice->getGuid() : std::string{},
				player);
		}
		ImGui::EndDisabled();
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
			ImGui::SetTooltip(profile && profile->builtIn ? "Duplicate built-in profile" : "Edit profile");
		}
		ImGui::PopStyleColor(12);
		ImGui::PopID();
	}

}
