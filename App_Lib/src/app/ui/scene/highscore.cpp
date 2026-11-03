#include "highscore.h"
#include "../imguiextra.h"

namespace app::ui::scene {

	void HighScore::imGuiUpdate(const DeltaTime& deltaTime) {
		auto labelColor = app::Configuration::getInstance().getLabelTextColor();
		const float scale = ImGui::GetUiScale();

		ImGui::PushFont(app::Configuration::getInstance().getImGuiSemiBoldFont(), 0.f);
		ImGui::TextColored(labelColor, "High Scores");
		ImGui::PopFont();

		ImGui::PushFont(app::Configuration::getInstance().getImGuiDefaultFont(), 0.f);
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, {8.f * scale, 6.f * scale});

		constexpr ImGuiTableFlags tableFlags =
			ImGuiTableFlags_BordersOuter |
			ImGuiTableFlags_BordersInnerH |
			ImGuiTableFlags_BordersInnerV |
			ImGuiTableFlags_RowBg |
			ImGuiTableFlags_SizingStretchProp |
			ImGuiTableFlags_PadOuterX |
			ImGuiTableFlags_NoSavedSettings;
		ImGui::Table("High Scores", 6, tableFlags, {ImGui::GetContentRegionAvail().x, 0.f}, [&]() {
			ImGui::TableSetupColumn("Rank", ImGuiTableColumnFlags_WidthFixed, 50.f * scale);
			ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("Points", ImGuiTableColumnFlags_WidthFixed, 75.f * scale);
			ImGui::TableSetupColumn("Rows", ImGuiTableColumnFlags_WidthFixed, 55.f * scale);
			ImGui::TableSetupColumn("Level", ImGuiTableColumnFlags_WidthFixed, 55.f * scale);
			ImGui::TableSetupColumn("Date", ImGuiTableColumnFlags_WidthFixed, 90.f * scale);
			ImGui::TableHeadersRow();

			int rankNbr = 1;
			for (const auto& highscore : results_) {
				ImGui::TableNextRow();
				if (highlightedRank_ == rankNbr) {
					ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImGuiCol_HeaderHovered));
				}
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
		highlightedRank_.reset();
		if (const auto* highScoreData = dynamic_cast<const HighScoreData*>(&sceneData)) {
			highlightedRank_ = highScoreData->highlightedRank;
		}
	}

}
