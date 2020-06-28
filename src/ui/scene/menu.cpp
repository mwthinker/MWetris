#include "menu.h"

#include "../../types.h"
#include "../../tetrisdata.h"
#include "../imguiextra.h"

namespace tetris::ui::scene {

	void Menu::imGuiUpdate(const std::chrono::high_resolution_clock::duration& deltaTime) {
		auto menuHeight = tetris::TetrisData::getInstance().getWindowBarHeight();

		ImGui::Bar([]() {});

		ImGui::PushFont(tetris::TetrisData::getInstance().getImGuiHeaderFont());
		ImGui::TextColored(tetris::TetrisData::getInstance().getLabelTextColor(), "MWetris");
		ImGui::PopFont();

		ImGui::PushButtonStyle();

		//ImGui::Indent(10.f);
		AddMenuButton("Play", Event::Play);
		AddMenuButton("Custom Play", Event::CustomPlay);
		AddMenuButton("Network Play", Event::NetworkPlay);
		AddMenuButton("Highscore", Event::HighScore);
		AddMenuButton("Settings", Event::Settings);
		AddMenuButton("Exit", Event::Exit);

		ImGui::PopButtonStyle();
	}

	void Menu::AddMenuButton(const std::string& label, Event event) {
		if (ImGui::Button(label.c_str())) {
			emitEvent<Event>(event);

			spdlog::info("[ImGuiWindow] pushed {}", label);
		}
		ImGui::Dummy(ImVec2{0.0f, 5.0f});
	}

}
