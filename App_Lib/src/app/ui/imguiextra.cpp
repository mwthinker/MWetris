#include "imguiextra.h"
#include "theme.h"

#include <IconsFontAwesome6.h>

#include <cstdarg>
#include <utility>

namespace ImGui {

	namespace {

		bool isMouseLeftClicked() {
			return ImGui::GetIO().MouseClicked[ImGuiMouseButton_Left];
		}

		bool isMouseRightClicked() {
			return ImGui::GetIO().MouseClicked[ImGuiMouseButton_Right];
		}

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
		const float scale = std::clamp(ImGui::GetFontSize() / 17.f, 0.5f, 2.f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {24.f * scale, 20.f * scale});
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {10.f * scale, 10.f * scale});
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {10.f * scale, 6.f * scale});
	}

	void PopContentStyle() {
		ImGui::PopStyleVar(3);
	}

	ImVec2 ModalSize(const ImVec2& logicalSize, float viewportFraction) {
		const float scale = std::clamp(ImGui::GetFontSize() / 17.f, 0.5f, 2.f);
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
		bool result = ImGui::Button(label, size);
		ImGui::PopStyleColor(3);
		PopButtonStyle();
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
