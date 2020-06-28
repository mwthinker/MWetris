#include "highscore.h"
#include "event.h"

namespace tetris::ui::scene {

	void HighScore::imGuiUpdate(const std::chrono::high_resolution_clock::duration& deltaTime) {
		auto menuHeight = tetris::TetrisData::getInstance().getWindowBarHeight();
		auto labelColor = tetris::TetrisData::getInstance().getLabelTextColor();

		ImGui::Bar([&]() {
			ImGui::PushButtonStyle();
			if (ImGui::Button("Menu", {100.5f, menuHeight})) {
				emitEvent<Event>(Event::Menu);
			}
			ImGui::PopButtonStyle();
		});

		ImGui::PushFont(tetris::TetrisData::getInstance().getImGuiHeaderFont());
		ImGui::TextColored(labelColor, "Highscore");
		ImGui::PopFont();

		ImGui::PushFont(tetris::TetrisData::getInstance().getImGuiDefaultFont());
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
			ImGui::Text("%i", highscore.points_); ImGui::NextColumn();
			ImGui::TextUnformatted(highscore.name_.c_str()); ImGui::NextColumn();
			ImGui::Text("%i", highscore.rows_); ImGui::NextColumn();
			ImGui::Text("%i", highscore.level_); ImGui::NextColumn();
			ImGui::TextUnformatted(highscore.date_.c_str()); ImGui::NextColumn();
		}
		ImGui::Separator();
		ImGui::PopFont();
	}

}
