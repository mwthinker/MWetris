#include "menu.h"

#include "types.h"
#include "configuration.h"
#include "ui/imguiextra.h"

namespace mwetris::ui::scene {

	void Menu::imGuiUpdate(const std::chrono::high_resolution_clock::duration& deltaTime) {
		auto menuHeight = mwetris::Configuration::getInstance().getWindowBarHeight();

		ImGui::Bar([&]() {
			ImGui::PushButtonStyle();
			ImGui::PopButtonStyle();
		});

		ImGui::PushFont(mwetris::Configuration::getInstance().getImGuiHeaderFont());
		ImGui::TextColored(mwetris::Configuration::getInstance().getLabelTextColor(), "MWetris");
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
			emitEvent(event);

			spdlog::info("[ImGuiWindow] pushed {}", label);
		}
		ImGui::Dummy(ImVec2{0.0f, 5.0f});
	}

}
