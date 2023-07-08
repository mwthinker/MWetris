#include "highscore.h"
#include "../imguiextra.h"

namespace mwetris::ui::scene {

	void HighScore::imGuiUpdate(const DeltaTime& deltaTime) {
		auto labelColor = mwetris::Configuration::getInstance().getLabelTextColor();

		ImGui::PushFont(mwetris::Configuration::getInstance().getImGuiHeaderFont());
		ImGui::TextColored(labelColor, "Highscore");
		ImGui::PopFont();

		ImGui::PushFont(mwetris::Configuration::getInstance().getImGuiDefaultFont());

		ImGui::Table("Highscore", 6, ImGuiTableFlags_Borders, {500, 0}, [&]() {
			ImGui::TableSetupColumn("Ranking", ImGuiTableColumnFlags_WidthFixed, 70);
			ImGui::TableSetupColumn("Points");
			ImGui::TableSetupColumn("Name");
			ImGui::TableSetupColumn("Rows");
			ImGui::TableSetupColumn("Level");
			ImGui::TableSetupColumn("Date");
			ImGui::TableHeadersRow();

			int rankNbr = 1;
			for (const auto& highscore : results_) {
				ImGui::TableNextRow();
				ImGui::TableNextColumn(); ImGui::Text("%i", rankNbr++);
				ImGui::TableNextColumn(); ImGui::Text("%i", highscore.points);
				ImGui::TableNextColumn(); ImGui::TextUnformatted(highscore.name.c_str());
				ImGui::TableNextColumn(); ImGui::Text("%i", highscore.rows);
				ImGui::TableNextColumn(); ImGui::Text("%i", highscore.level);
				ImGui::TableNextColumn(); ImGui::Text(highscore.lastPlayed);
			}
		});

		ImGui::PopFont();
	}

	void HighScore::switchedTo(const SceneData& sceneData) {
		results_ = game::loadHighScore();
	}

}
