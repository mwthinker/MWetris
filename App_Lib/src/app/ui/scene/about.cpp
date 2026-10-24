#include "about.h"
#include "../imguiextra.h"

#include <SDL3/SDL_misc.h>
#include <spdlog/spdlog.h>

#include <string>
#include <string_view>

namespace app::ui::scene {
	namespace {
		void detailRow(const char* label, std::string_view value, bool selectable = false) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted(label);
			ImGui::TableNextColumn();
			if (selectable) {
				auto text = std::string{value};
				ImGui::SetNextItemWidth(-FLT_MIN);
				ImGui::PushID(label);
				ImGui::InputText("##Value", &text, ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_AutoSelectAll);
				ImGui::PopID();
			} else {
				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted(value.data(), value.data() + value.size());
			}
		}

		void detailsTable(const char* id, std::invocable auto&& rows) {
			if (ImGui::BeginTable(id, 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoSavedSettings)) {
				ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 170.f);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
				rows();
				ImGui::EndTable();
			}
		}
	}

	void About::imGuiUpdate(const DeltaTime&) {
		auto labelColor = app::Configuration::getInstance().getLabelTextColor();

		ImGui::PushFont(app::Configuration::getInstance().getImGuiSemiBoldFont(), 0.f);
		ImGui::TextColored(labelColor, "MWetris");
		ImGui::PopFont();

		ImGui::PushFont(app::Configuration::getInstance().getImGuiDefaultFont(), 0.f);

		ImGui::Spacing();
		ImGui::TextUnformatted("An open-source, Tetris-inspired game created by Marcus Welander (mwthinker).");
		constexpr auto Url = "https://github.com/mwthinker/MWetris";
		if (ImGui::LinkButton(Url)) {
			if (SDL_OpenURL(Url) != 0) {
				spdlog::error("[SceneStateMachine] Failed to open URL: {}", SDL_GetError());
			}
		}

		ImGui::Spacing();
		ImGui::SeparatorText("Release");
		detailsTable("ReleaseDetails", [&]() {
			detailRow("Version", "v" PROJECT_VERSION);
			detailRow("License", "MIT");
			detailRow("Source date", GIT_DATE);
			detailRow("Dependencies date", VCPKG_DATE);
		});

		ImGui::Spacing();
		ImGui::SeparatorText("Build identifiers");
		detailsTable("BuildIdentifiers", [&]() {
			detailRow("Git commit", GIT_VERSION, true);
			detailRow("vcpkg commit", VCPKG_HASH, true);
			detailRow("Build GUID", MWETRIS_GUID, true);
		});

		ImGui::PopFont();
	}

}
