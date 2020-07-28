#include "highscore.h"
#include "event.h"

namespace mwetris::ui::scene {

	void HighScore::imGuiUpdate(const std::chrono::high_resolution_clock::duration& deltaTime) {
		auto menuHeight = mwetris::TetrisData::getInstance().getWindowBarHeight();
		auto labelColor = mwetris::TetrisData::getInstance().getLabelTextColor();

		ImGui::Bar([&]() {
			ImGui::PushButtonStyle();
			if (ImGui::Button("Menu", {100.5f, menuHeight})) {
				emitEvent<Event>(Event::Menu);
			}
			ImGui::PopButtonStyle();
		});

		ImGui::PushFont(mwetris::TetrisData::getInstance().getImGuiHeaderFont());
		ImGui::TextColored(labelColor, "Highscore");
		ImGui::PopFont();

		ImGui::PushFont(mwetris::TetrisData::getInstance().getImGuiDefaultFont());
		ImGui::Columns(6, "Highscore");
		ImGui::Separator();
		ImGui::Text("Ranking"); ImGui::NextColumn();
		ImGui::Text("Points"); ImGui::NextColumn();
		ImGui::Text("Name"); ImGui::NextColumn();
		ImGui::Text("Rows"); ImGui::NextColumn();
		ImGui::Text("Level"); ImGui::NextColumn();
		ImGui::Text("Date"); ImGui::NextColumn();
		ImGui::Separator();

		auto highscores = TetrisData::getInstance().getHighscoreRecordVector();

		int rankNbr = 1;
		for (const auto& highscore : highscores) {
			ImGui::Text("%i", rankNbr++); ImGui::NextColumn();
			ImGui::Text("%i", highscore.points); ImGui::NextColumn();
			ImGui::TextUnformatted(highscore.name.c_str()); ImGui::NextColumn();
			ImGui::Text("%i", highscore.rows); ImGui::NextColumn();
			ImGui::Text("%i", highscore.level); ImGui::NextColumn();
			ImGui::TextUnformatted(highscore.date.c_str()); ImGui::NextColumn();
		}
		ImGui::Separator();
		ImGui::PopFont();
	}

}
