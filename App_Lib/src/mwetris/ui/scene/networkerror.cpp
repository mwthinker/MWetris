#include "networkerror.h"
#include "../imguiextra.h"

#include <SDL_misc.h>
#include <spdlog/spdlog.h>

namespace mwetris::ui::scene {

	void NetworkError::imGuiUpdate(const DeltaTime& deltaTime) {
		auto labelColor = mwetris::Configuration::getInstance().getLabelTextColor();

		ImGui::PushFont(mwetris::Configuration::getInstance().getImGuiHeaderFont());
		ImGui::TextColored(labelColor, "Network error");
		ImGui::PopFont();

		ImGui::PushFont(mwetris::Configuration::getInstance().getImGuiDefaultFont());

		ImGui::NewLine();
		ImGui::Text("Aborted");
		ImGui::NewLine();

		ImGui::PopFont();
	}

}
