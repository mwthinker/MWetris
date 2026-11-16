#include "imguiextra.h"
#include "theme.h"

#include <IconsFontAwesome6.h>

#include <cstdarg>
#include <utility>
#include <optional>

namespace ImGui {

	namespace {
		
		std::optional<app::audio::SoundEffect> frameSound;

		bool isMouseLeftClicked() {
			return ImGui::GetIO().MouseClicked[ImGuiMouseButton_Left];
		}

		bool isMouseRightClicked() {
			return ImGui::GetIO().MouseClicked[ImGuiMouseButton_Right];
		}

		bool isViewportVisible(const ImGuiViewport& viewport, const ImGuiPlatformIO& platform) {
			constexpr float MinimumVisibleWidth = 64.f;
			constexpr float MinimumVisibleHeight = 32.f;
			for (const auto& monitor : platform.Monitors) {
				const auto overlapWidth = std::min(viewport.Pos.x + viewport.Size.x, monitor.WorkPos.x + monitor.WorkSize.x)
					- std::max(viewport.Pos.x, monitor.WorkPos.x);
				const auto overlapHeight = std::min(viewport.Pos.y + viewport.Size.y, monitor.WorkPos.y + monitor.WorkSize.y)
					- std::max(viewport.Pos.y, monitor.WorkPos.y);
				if (overlapWidth >= std::min(MinimumVisibleWidth, viewport.Size.x)
					&& overlapHeight >= std::min(MinimumVisibleHeight, viewport.Size.y)) {
					return true;
				}
			}
			return false;
		}

	}

	void BeginAudioFrame() {
		frameSound.reset();
	}

	void ReportAudioSound(app::audio::SoundEffect effect) {
		if (!frameSound) frameSound = effect;
	}

	bool ConsumeAudioSound(app::audio::SoundEffect& effect) {
		if (!frameSound) return false;
		effect = *frameSound;
		frameSound.reset();
		return true;
	}

	void PushButtonStyle() {
		ImGui::PushFont(app::Configuration::getInstance().getImGuiMediumFont(), 0.f);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
		ImGui::PushStyleColor(ImGuiCol_Text, app::Configuration::getInstance().getButtonTextColor().toImU32());
		ImGui::PushStyleColor(ImGuiCol_Border, app::Configuration::getInstance().getButtonBorderColor().toImU32());
		ImGui::PushStyleColor(ImGuiCol_Button, app::Configuration::getInstance().getButtonBackgroundColor().toImU32());
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, app::Configuration::getInstance().getButtonHoverColor().toImU32());
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, app::Configuration::getInstance().getButtonFocusColor().toImU32());
	}

	void PopButtonStyle() {
		ImGui::PopStyleColor(5);
		ImGui::PopStyleVar();
		ImGui::PopFont();
	}

	void PushContentStyle() {
		const float scale = GetUiScale();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {24.f * scale, 20.f * scale});
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {10.f * scale, 10.f * scale});
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {10.f * scale, 6.f * scale});
	}

	void PopContentStyle() {
		ImGui::PopStyleVar(3);
	}

	void EnsurePlatformWindowsVisible() {
		auto& platform = ImGui::GetPlatformIO();
		if (!platform.Platform_SetWindowPos) {
			return;
		}

		const auto* mainViewport = ImGui::GetMainViewport();
		for (auto* viewport : platform.Viewports) {
			if (viewport == mainViewport
				|| !viewport->PlatformWindowCreated
				|| (viewport->Flags & (ImGuiViewportFlags_IsMinimized | ImGuiViewportFlags_NoInputs)) != 0) {
				continue;
			}

			if (isViewportVisible(*viewport, platform)) {
				continue;
			}

			platform.Platform_SetWindowPos(viewport, {
				mainViewport->WorkPos.x + 60.f,
				mainViewport->WorkPos.y + 60.f
			});
		}
	}

	float GetUiScale() {
		return std::clamp(ImGui::GetFontSize() / 17.f, 0.5f, 2.f);
	}

	ImVec2 ModalSize(const ImVec2& logicalSize, float viewportFraction) {
		const float scale = GetUiScale();
		const auto viewportSize = ImGui::GetMainViewport()->WorkSize;
		return {
			std::min(logicalSize.x * scale, viewportSize.x * viewportFraction),
			std::min(logicalSize.y * scale, viewportSize.y * viewportFraction)
		};
	}

	void SetNextModalWindow(const ImVec2& logicalSize) {
		auto viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::SetNextWindowPos(viewport->GetWorkCenter(), ImGuiCond_Always, {0.5f, 0.5f});
		ImGui::SetNextWindowSize(ImGui::ModalSize(logicalSize), ImGuiCond_Always);
	}

	float ModalFooterHeight() {
		return ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y * 2.f + 1.f;
	}

	bool BeginModalBody(const char* id, bool border) {
		return ImGui::BeginChild(id, {0.f, -ModalFooterHeight()}, border);
	}

	void EndModalBody() {
		ImGui::EndChild();
		ImGui::Separator();
		ImGui::Spacing();
	}

	bool MediumButton(const char* label, const ImVec2& size) {
		PushButtonStyle();
		bool result = ImGui::Button(label, size);
		PopButtonStyle();
		return result;
	}

	bool CloseIconButton(const ImVec2& size) {
		ImGui::PushFont(app::Configuration::getInstance().getImGuiDefaultFont(), 0.f);
		const auto position = ImGui::GetCursorScreenPos();
		const ImVec2 buttonSize{
			size.x > 0.f ? size.x : ImGui::GetFrameHeight(),
			size.y > 0.f ? size.y : ImGui::GetFrameHeight()
		};
		const bool result = ImGui::InvisibleButton("##Close", buttonSize);
		const bool hovered = ImGui::IsItemHovered();
		const bool active = ImGui::IsItemActive();
		auto* drawList = ImGui::GetWindowDrawList();

		if (hovered || active) {
			const auto backgroundColor = active
				? app::Configuration::getInstance().getButtonPushColor().toImU32()
				: app::Configuration::getInstance().getButtonHoverColor().toImU32();
			drawList->AddRectFilled(
				position,
				{position.x + buttonSize.x, position.y + buttonSize.y},
				backgroundColor,
				std::min(buttonSize.x, buttonSize.y) * 0.2f);
		}

		constexpr ImWchar xmarkCodepoint = 0xf00d;
		const auto* glyph = ImGui::GetFontBaked()->FindGlyphNoFallback(xmarkCodepoint);
		const auto iconSize = ImGui::CalcTextSize(ICON_FA_XMARK);
		const ImVec2 iconPosition = glyph
			? ImVec2{
				position.x + buttonSize.x * 0.5f - (glyph->X0 + glyph->X1) * 0.5f,
				position.y + buttonSize.y * 0.5f - (glyph->Y0 + glyph->Y1) * 0.5f
			}
			: ImVec2{
				position.x + (buttonSize.x - iconSize.x) * 0.5f,
				position.y + (buttonSize.y - iconSize.y) * 0.5f
			};
		const auto iconColor = hovered || active
			? app::Configuration::getInstance().getButtonTextColor().toImU32()
			: ImGui::GetColorU32(ImGuiCol_TextDisabled);
		drawList->AddText(iconPosition, iconColor, ICON_FA_XMARK);
		ImGui::PopFont();
		return result;
	}

	bool MediumSmallButton(const char* label) {
		PushButtonStyle();
		bool result = ImGui::SmallButton(label);
		PopButtonStyle();
		return result;
	}

	bool LinkButton(const char* label) {
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.f);
		ImGui::PushStyleColor(ImGuiCol_Text, app::ui::theme::Link.toImU32());
		ImGui::PushStyleColor(ImGuiCol_Button, sdl::color::Transparent.toImU32());
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, sdl::color::Transparent.toImU32());
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, sdl::color::Transparent.toImU32());
		const bool result = ImGui::Button(label);
		ImGui::PopStyleColor(4);
		ImGui::PopStyleVar();

		if (ImGui::IsItemHovered()) {
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
			const auto min = ImGui::GetItemRectMin();
			const auto max = ImGui::GetItemRectMax();
			const float padding = ImGui::GetStyle().FramePadding.x;
			ImGui::GetWindowDrawList()->AddLine(
				{min.x + padding, max.y - ImGui::GetStyle().FramePadding.y},
				{max.x - padding, max.y - ImGui::GetStyle().FramePadding.y},
				app::ui::theme::LinkHovered.toImU32());
		}

		return result;
	}

	bool SemiBoldCollapsingHeader(const char* label, ImGuiTreeNodeFlags flags) {
		ImGui::PushFont(app::Configuration::getInstance().getImGuiSemiBoldFont(), 0.f);
		bool result = ImGui::CollapsingHeader(label, flags);
		ImGui::PopFont();
		return result;
	}

	void SecondaryText(const char* format, ...) {
		ImGui::PushFont(app::Configuration::getInstance().getImGuiRegularSmallFont(), 0.f);
		va_list args;
		va_start(args, format);
		ImGui::TextDisabledV(format, args);
		va_end(args);
		ImGui::PopFont();
	}

	void DeviceLabel(const app::game::Device& device, bool disabled) {
		const auto icon = device.getKind() == app::game::DeviceKind::Keyboard
			? ICON_FA_KEYBOARD
			: ICON_FA_GAMEPAD;
		ImGui::PushFont(app::Configuration::getInstance().getImGuiDefaultFont(), 0.f);
		ImGui::PushStyleColor(ImGuiCol_Text, disabled
			? ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled)
			: static_cast<ImVec4>(device.getColor()));
		ImGui::TextUnformatted(icon);
		ImGui::PopStyleColor();
		ImGui::PopFont();
		ImGui::SameLine(0.f, ImGui::GetStyle().ItemInnerSpacing.x);
		if (disabled) {
			ImGui::TextDisabled("%s", device.getName().c_str());
		} else {
			ImGui::TextUnformatted(device.getName().c_str());
		}
	}

	namespace {

		ImWchar deviceIconCodepoint(const app::game::Device& device) {
			return device.getKind() == app::game::DeviceKind::Keyboard ? 0xf11c : 0xf11b;
		}

		float deviceIconRightEdge(ImFont* font, float fontSize, const app::game::Device& device, const char* icon) {
			if (const auto* glyph = font->GetFontBaked(fontSize)->FindGlyphNoFallback(deviceIconCodepoint(device))) {
				return glyph->X1;
			}
			return font->CalcTextSizeA(fontSize, FLT_MAX, 0.f, icon).x;
		}

		float deviceLabelSpacing(float fontSize) {
			return fontSize * 0.15f;
		}

		bool deviceSelectable(const app::game::Device& device, bool selected) {
			const auto position = ImGui::GetCursorScreenPos();
			const auto height = ImGui::GetFrameHeight();
			ImGui::PushID(device.getGuid().c_str());
			const bool activated = ImGui::Selectable("##Device", selected, 0, {0.f, height});
			ImGui::PopID();

			auto* defaultFont = app::Configuration::getInstance().getImGuiDefaultFont();
			auto* textFont = ImGui::GetFont();
			const auto fontSize = ImGui::GetFontSize();
			const auto icon = device.getKind() == app::game::DeviceKind::Keyboard
				? ICON_FA_KEYBOARD
				: ICON_FA_GAMEPAD;
			const float iconRightEdge = deviceIconRightEdge(defaultFont, fontSize, device, icon);
			const ImVec2 textPosition{
				position.x + ImGui::GetStyle().FramePadding.x,
				position.y + (height - fontSize) * 0.5f,
			};
			auto* drawList = ImGui::GetWindowDrawList();
			drawList->AddText(defaultFont, fontSize, textPosition, device.getColor().toImU32(), icon);
			drawList->AddText(textFont, fontSize,
				{textPosition.x + iconRightEdge + deviceLabelSpacing(fontSize), textPosition.y},
				ImGui::GetColorU32(ImGuiCol_Text), device.getName().c_str());
			return activated;
		}

		bool beginDeviceCombo(const char* label, const app::game::Device* device,
			std::string_view suffix, const char* emptyPreview) {

			const auto position = ImGui::GetCursorScreenPos();
			auto* textFont = ImGui::GetFont();
			const float fontSize = ImGui::GetFontSize();
			const bool opened = ImGui::BeginCombo(label, device ? "" : emptyPreview);
			if (!device) {
				return opened;
			}

			const auto icon = device->getKind() == app::game::DeviceKind::Keyboard
				? ICON_FA_KEYBOARD
				: ICON_FA_GAMEPAD;
			auto* iconFont = app::Configuration::getInstance().getImGuiDefaultFont();
			const float iconRightEdge = deviceIconRightEdge(iconFont, fontSize, *device, icon);
			const ImVec2 textPosition{
				position.x + ImGui::GetStyle().FramePadding.x,
				position.y + ImGui::GetStyle().FramePadding.y,
			};
			auto* drawList = ImGui::GetWindowDrawList();
			drawList->AddText(iconFont, fontSize, textPosition, device->getColor().toImU32(), icon);
			auto name = device->getName() + std::string{suffix};
			drawList->AddText(textFont, fontSize,
				{textPosition.x + iconRightEdge + deviceLabelSpacing(fontSize), textPosition.y},
				ImGui::GetColorU32(ImGuiCol_Text), name.c_str());
			return opened;
		}

	}

	bool DeviceCombo(const char* label, std::span<const app::game::DevicePtr> devices,
		app::game::DevicePtr& selectedDevice, const char* emptyPreview) {

		bool changed = false;
		if (!selectedDevice && !devices.empty()) {
			selectedDevice = devices.front();
			changed = true;
		}
		const auto suffix = selectedDevice && !selectedDevice->isConnected() ? " (disconnected)" : "";
		if (beginDeviceCombo(label, selectedDevice.get(), suffix, emptyPreview)) {
			for (const auto& device : devices) {
				const bool selected = selectedDevice && selectedDevice->getGuid() == device->getGuid();
				if (deviceSelectable(*device, selected)) {
					selectedDevice = device;
					changed = true;
				}
				if (selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		return changed;
	}

	void Text(const std::chrono::year_month_day& ymd) {
		ImGui::Text("%d-%02d-%02d", static_cast<int>(ymd.year()), static_cast<int>(static_cast<unsigned>(ymd.month())), static_cast<int>(static_cast<unsigned>(ymd.day())));
	}

	void TextWithBackgroundColor(const char* str, sdl::Color color, const ImVec2& padding) {
		auto pos = ImGui::GetCursorScreenPos();
		auto size = ImGui::CalcTextSize(str);

		ImGui::GetWindowDrawList()->AddRectFilled({pos.x - padding.x, pos.y - padding.y}, ImVec2{pos.x + size.x + padding.x, pos.y + size.y + padding.y}, color.toImU32());
		ImGui::Text("%s", str);
	}

	void TextWithBackgroundColor(int nbr, sdl::Color color, const ImVec2& padding) {
		auto pos = ImGui::GetCursorScreenPos();
		auto size = ImGui::CalcTextSize("%d");

		ImGui::GetWindowDrawList()->AddRectFilled({pos.x - padding.x, pos.y - padding.y}, ImVec2{pos.x + size.x + padding.x, pos.y + size.y + padding.y}, color.toImU32());
		ImGui::Text("%d", nbr);
	}

	void IgnoreCursor(std::invocable auto&& t) {
		auto pos = ImGui::GetCursorPos();
		t();
		ImGui::SetCursorPos(pos);
	}

	bool ConfirmationButton(const char* label, const ImVec2& size) {
		PushButtonStyle();
		ImGui::PushStyleColor(ImGuiCol_Button, app::ui::theme::Success.toImU32());
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, app::ui::theme::SuccessHovered.toImU32());
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, app::ui::theme::SuccessActive.toImU32());
		ImGui::PushStyleColor(ImGuiCol_Text, app::ui::theme::SuccessButtonText.toImU32());
		bool result = ImGui::Button(label, size);
		ImGui::PopStyleColor(4);
		PopButtonStyle();
		if (result) ReportAudioSound(app::audio::SoundEffect::UiConfirm);
		return result;
	}

	bool AbortButton(const char* label, const ImVec2& size) {
		PushButtonStyle();
		ImGui::PushStyleColor(ImGuiCol_Button, app::ui::theme::Danger.toImU32());
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, app::ui::theme::DangerHovered.toImU32());
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, app::ui::theme::DangerActive.toImU32());
		bool result = ImGui::Button(label, size);
		ImGui::PopStyleColor(3);
		PopButtonStyle();
		return result;
	}

}
