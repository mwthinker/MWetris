#include "highscore.h"
#include "../imguiextra.h"

namespace app::ui::scene {

	void HighScore::imGuiUpdate(const DeltaTime& deltaTime) {
		auto labelColor = app::Configuration::getInstance().getLabelTextColor();

		ImGui::PushFont(app::Configuration::getInstance().getImGuiSemiBoldFont(), 0.f);
		ImGui::TextColored(labelColor, "High Scores");
		ImGui::PopFont();

		ImGui::PushFont(app::Configuration::getInstance().getImGuiDefaultFont(), 0.f);
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, {8.f, 6.f});

		constexpr ImGuiTableFlags tableFlags =
			ImGuiTableFlags_BordersOuter |
			ImGuiTableFlags_BordersInnerH |
			ImGuiTableFlags_BordersInnerV |
			ImGuiTableFlags_RowBg |
			ImGuiTableFlags_SizingStretchProp |
			ImGuiTableFlags_PadOuterX |
			ImGuiTableFlags_NoSavedSettings;
		ImGui::Table("High Scores", 6, tableFlags, {ImGui::GetContentRegionAvail().x, 0.f}, [&]() {
			ImGui::TableSetupColumn("Rank", ImGuiTableColumnFlags_WidthFixed, 50.f);
			ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("Points", ImGuiTableColumnFlags_WidthFixed, 75.f);
			ImGui::TableSetupColumn("Rows", ImGuiTableColumnFlags_WidthFixed, 55.f);
			ImGui::TableSetupColumn("Level", ImGuiTableColumnFlags_WidthFixed, 55.f);
			ImGui::TableSetupColumn("Date", ImGuiTableColumnFlags_WidthFixed, 90.f);
			ImGui::TableHeadersRow();

			int rankNbr = 1;
			for (const auto& highscore : results_) {
				ImGui::TableNextRow();
				ImGui::TableNextColumn(); ImGui::Text("%i", rankNbr++);
				ImGui::TableNextColumn(); ImGui::TextUnformatted(highscore.name.c_str());
				if (highscore.points > 0) {
					ImGui::TableNextColumn(); ImGui::Text("%i", highscore.points);
					ImGui::TableNextColumn(); ImGui::Text("%i", highscore.rows);
					ImGui::TableNextColumn(); ImGui::Text("%i", highscore.level);
					ImGui::TableNextColumn(); ImGui::Text(highscore.lastPlayed);
				} else {
					ImGui::TableNextColumn(); ImGui::Text("");
					ImGui::TableNextColumn(); ImGui::Text("");
					ImGui::TableNextColumn(); ImGui::Text("");
					ImGui::TableNextColumn(); ImGui::Text("");
				}
			}
		});

		ImGui::PopStyleVar();
		ImGui::PopFont();
	}

	void HighScore::switchedTo(const SceneData& sceneData) {
		results_ = game::loadHighScore();
	}

}
