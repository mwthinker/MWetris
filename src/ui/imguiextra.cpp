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
		ImGui::PushFont(mwetris::TetrisData::getInstance().getImGuiButtonFont());
		ImGui::PushStyleColor(ImGuiCol_Text, mwetris::TetrisData::getInstance().getButtonTextColor().toImU32());
		ImGui::PushStyleColor(ImGuiCol_Border, mwetris::TetrisData::getInstance().getButtonBorderColor().toImU32());
		ImGui::PushStyleColor(ImGuiCol_Button, mwetris::TetrisData::getInstance().getButtonBackgroundColor().toImU32());
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, mwetris::TetrisData::getInstance().getButtonHoverColor().toImU32());
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, mwetris::TetrisData::getInstance().getButtonFocusColor().toImU32());
	}

	void PopButtonStyle() {
		ImGui::PopStyleColor(5);
		ImGui::PopFont();
	}

	bool ManButton(const char* idStr, int& nbr, int max, const sdl::TextureView& noMan,
		const sdl::TextureView& man, const glm::vec2& size, const sdl::Color& color) {
		
		glm::vec2 p = GetCursorScreenPos();

		ImGui::PushID(idStr);
		int buttons = (nbr > 0 ? nbr : 1);
		ImGui::InvisibleButton("", {size.x * buttons, size.y});
			
		glm::vec2 delta{};
		glm::vec2 scaledSize = size;
			
		bool pushed = false;
		if (ImGui::IsItemHovered()) {
			if (ImGui::IsAnyMouseDown()) {
				if (isMouseRightClicked()) {
					nbr = (nbr + 4) % (max + 1);
					pushed = true;
				} else if (isMouseLeftClicked()) {
					nbr = (nbr + 1) % (max + 1);
					pushed = true;
				}
			}

			scaledSize *= 1.2f;
			delta = 0.1f * scaledSize;
		}
		ImGui::PopID();

		auto drawList = ImGui::GetWindowDrawList();
		drawList->PushTextureID((ImTextureID)(intptr_t) noMan);
		buttons = (nbr > 0 ? nbr : 2);
		drawList->PrimReserve(6 * buttons, 4 * buttons);
		
		for (int i = 1; i <= nbr || nbr == 0 && i == 1; ++i) {
			float x = p.x;
			if (nbr != 0) {
				x = p.x + size.x * (i -1);
				Helper::AddImageQuad(man, {x - delta.x, p.y - delta.y}, scaledSize, color);
			} else {
				Helper::AddImageQuad(man, {x - delta.x, p.y - delta.y}, scaledSize, color);
				Helper::AddImageQuad(noMan, {x - delta.x, p.y - delta.y}, scaledSize, color);
			}
		}

		drawList->PopTextureID();
		return pushed;
	}

	void LoadingBar(const sdl::Color& color1, const sdl::Color& color2, const ImVec2& size) {
		ImGui::Dummy(size);
		auto pos = ImGui::GetCursorPos();
		ImGui::GetWindowDrawList()->AddRectFilled(pos, { pos.x + size.x, pos.y + size.y }, color1.toImU32());

		static float sign = 1;   // Obs!!!! Only possible with one loading bar at a time.
		static float x = pos.x;
		
		const float speed = 100.f;
		x += sign * ImGui::GetIO().DeltaTime * speed;
		const float width = size.x * 0.3f;
		if (x > pos.x + size.x - width) {
			sign = -1.f;
			x = pos.x + size.x - width;
		}
		if (x < pos.x) {
			sign = 1.f;
			x = pos.x;
		}
		ImGui::GetWindowDrawList()->AddRectFilled({ x, pos.y }, { x + width, pos.y + size.y }, color2.toImU32());
	}

}
