#include "about.h"
#include "../imguiextra.h"

#include <SDL3/SDL_misc.h>
#include <spdlog/spdlog.h>

namespace app::ui::scene {

	void About::imGuiUpdate(const DeltaTime& deltaTime) {
		auto labelColor = app::Configuration::getInstance().getLabelTextColor();

		ImGui::PushFont(app::Configuration::getInstance().getImGuiHeaderFont(), 0.f);
		ImGui::TextColored(labelColor, "MWetris");
		ImGui::PopFont();

		ImGui::PushFont(app::Configuration::getInstance().getImGuiDefaultFont(), 0.f);

		ImGui::NewLine();
		ImGui::Text("MWetris is created by Marcus Welander (mwthinker).");
		ImGui::NewLine();
		ImGui::Text("An open source inspired Tetris game.");
		constexpr auto Url = "https://github.com/mwthinker/MWetris";
		if (ImGui::Button(Url)) {
			if (SDL_OpenURL(Url) != 0) {
				spdlog::error("[SceneStateMachine] Failed to open URL: {}", SDL_GetError());
			}
		}
		ImGui::NewLine();
		ImGui::Text("Version:  v%s", PROJECT_VERSION);
		ImGui::Text("Git hash: %s  date:  %s", GIT_VERSION, GIT_DATE);
		ImGui::Text("Vcpkg hash: %s  date:  %s", VCPKG_HASH, VCPKG_DATE);
		ImGui::NewLine();
		ImGui::Text("GUID: %s", MWETRIS_GUID);

		ImGui::PopFont();
	}

}
