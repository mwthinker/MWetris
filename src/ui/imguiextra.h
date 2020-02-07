#ifndef MWETRIS_IMGUIEXTRA_H
#define MWETRIS_IMGUIEXTRA_H

#include "../types.h"

#include <sdl/imguiwindow.h>

#include <sdl/sprite.h>

namespace ImGui {

	template <class T>
	bool Bar(float height, T&& t) {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, tetris::TetrisData::getInstance().getWindowBarColor().toImU32());
		ImGui::ChildWindow("Bar", ImVec2{ImGui::GetWindowWidth(), height}, [&]() {
			ImGui::GetWindowDrawList()->AddRectFilled({0, 0},
				ImGui::GetWindowSize(),
				tetris::TetrisData::getInstance().getWindowBarColor().toImU32());
			t();
		});
		
		//ImGui::Dummy({0.f, tetris::TetrisData::getInstance().getWindowBarHeight()});
		ImGui::SetCursorPos({0.f, tetris::TetrisData::getInstance().getWindowBarHeight()});
		ImGui::PopStyleColor();
		ImGui::PopStyleVar(2);
		return true;
	}

	bool ManButton(const char* idStr, int& nbr, int max, const sdl::TextureView& noMan,
		const sdl::TextureView& man, const glm::vec2& size, const sdl::Color& color = sdl::WHITE);

	void LoadingBar(const sdl::Color& color1 = {0.8f, 0.8f, 0.8f, 1.f},
		const sdl::Color& color2 = {0.5f, 0.5f, 0.5f, 1.f},
		const ImVec2& size = {150.f, 25.f});

} // Namespace ImGui.

#endif // MWETRIS_IMGUIEXTRA_H
