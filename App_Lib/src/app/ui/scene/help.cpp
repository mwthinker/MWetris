#include "help.h"

#include "configuration.h"
#include "ui/imguiextra.h"

namespace app::ui::scene {

	namespace {
		void heading(const char* text) {
			ImGui::PushFont(Configuration::getInstance().getImGuiSemiBoldFont(), 0.f);
			ImGui::TextUnformatted(text);
			ImGui::PopFont();
		}

		void shortcut(const char* keys, const char* action) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::TextUnformatted(keys);
			ImGui::TableNextColumn();
			ImGui::TextUnformatted(action);
		}
	}

	void HowToPlay::imGuiUpdate(const DeltaTime&) {
		heading("How to Play");
		ImGui::Separator();
		heading("Goal");
		ImGui::TextWrapped("Arrange falling blocks to complete horizontal lines. Completed lines disappear and award points. The game ends when blocks reach the top of the board.");
		ImGui::Spacing();
		heading("Actions");
		ImGui::BulletText("Move left or right");
		ImGui::BulletText("Soft drop to move down faster");
		ImGui::BulletText("Rotate the falling block");
		ImGui::BulletText("Hard drop to place the block immediately");
		ImGui::Spacing();
		heading("Controls");
		ImGui::TextWrapped("Input mappings and movement profiles can be changed under Settings > Controls.");
	}

	void KeyboardShortcuts::imGuiUpdate(const DeltaTime&) {
		heading("Keyboard Shortcuts");
		ImGui::Separator();
		if (ImGui::BeginTable("Shortcuts", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingStretchProp)) {
			ImGui::TableSetupColumn("Shortcut", ImGuiTableColumnFlags_WidthFixed, 130.f);
			ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableHeadersRow();
			shortcut("F1", "Start or resume single player");
			shortcut("P / Pause", "Pause or resume the current game");
			shortcut("F5", "Restart the current game");
			shortcut("Escape", "Pause the game or close a dialog");
			ImGui::EndTable();
		}
	}

}
