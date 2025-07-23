#include "highscore.h"
#include "../imguiextra.h"

namespace app::ui::scene {

	void HighScore::imGuiUpdate(const DeltaTime& deltaTime) {
		auto labelColor = app::Configuration::getInstance().getLabelTextColor();

		ImGui::PushFont(app::Configuration::getInstance().getImGuiHeaderFont(), 0.f);
		ImGui::TextColored(labelColor, "Highscore");
		ImGui::PopFont();

		ImGui::PushFont(app::Configuration::getInstance().getImGuiDefaultFont(), 0.f);

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
				if (highscore.points > 0) {
					ImGui::TableNextColumn(); ImGui::Text("%i", highscore.rows);
					ImGui::TableNextColumn(); ImGui::Text("%i", highscore.level);
					ImGui::TableNextColumn(); ImGui::Text(highscore.lastPlayed);
				} else {
					ImGui::TableNextColumn(); ImGui::Text("");
					ImGui::TableNextColumn(); ImGui::Text("");
					ImGui::TableNextColumn(); ImGui::Text("");
				}
			}
		});

		ImGui::PopFont();
	}

	void HighScore::switchedTo(const SceneData& sceneData) {
		results_ = game::loadHighScore();
	}

}
