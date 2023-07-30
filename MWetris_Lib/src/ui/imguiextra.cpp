#include "imguiextra.h"

#include <sdl/sprite.h>
#include <sdl/imguiauxiliary.h>

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
		ImGui::PushFont(mwetris::Configuration::getInstance().getImGuiButtonFont());
		ImGui::PushStyleColor(ImGuiCol_Text, mwetris::Configuration::getInstance().getButtonTextColor().toImU32());
		ImGui::PushStyleColor(ImGuiCol_Border, mwetris::Configuration::getInstance().getButtonBorderColor().toImU32());
		ImGui::PushStyleColor(ImGuiCol_Button, mwetris::Configuration::getInstance().getButtonBackgroundColor().toImU32());
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, mwetris::Configuration::getInstance().getButtonHoverColor().toImU32());
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, mwetris::Configuration::getInstance().getButtonFocusColor().toImU32());
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

}
