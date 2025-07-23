#include "imguiextra.h"

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
		ImGui::PushFont(app::Configuration::getInstance().getImGuiButtonFont(), 0.f);
		ImGui::PushStyleColor(ImGuiCol_Text, app::Configuration::getInstance().getButtonTextColor().toImU32());
		ImGui::PushStyleColor(ImGuiCol_Border, app::Configuration::getInstance().getButtonBorderColor().toImU32());
		ImGui::PushStyleColor(ImGuiCol_Button, app::Configuration::getInstance().getButtonBackgroundColor().toImU32());
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, app::Configuration::getInstance().getButtonHoverColor().toImU32());
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, app::Configuration::getInstance().getButtonFocusColor().toImU32());
	}

	void PopButtonStyle() {
		ImGui::PopStyleColor(5);
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
		ImGui::PushStyleColor(ImGuiCol_Button, sdl::color::Green);
		bool result = ImGui::Button(label, size);
		ImGui::PopStyleColor();
		return result;
	}

	bool AbortButton(const char* label, const ImVec2& size) {
		ImGui::PushStyleColor(ImGuiCol_Button, sdl::color::Red);
		bool result = ImGui::Button(label, size);
		ImGui::PopStyleColor();
		return result;
	}

}
