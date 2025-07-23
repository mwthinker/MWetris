#include "networkerror.h"
#include "../imguiextra.h"

#include <SDL3/SDL_misc.h>
#include <spdlog/spdlog.h>

namespace app::ui::scene {

	void NetworkError::imGuiUpdate(const DeltaTime& deltaTime) {
		auto labelColor = app::Configuration::getInstance().getLabelTextColor();

		ImGui::PushFont(app::Configuration::getInstance().getImGuiHeaderFont(), 0.f);
		ImGui::TextColored(labelColor, "Network error");
		ImGui::PopFont();

		ImGui::PushFont(app::Configuration::getInstance().getImGuiDefaultFont(), 0.f);

		ImGui::NewLine();
		ImGui::Text("Aborted");
		ImGui::NewLine();

		ImGui::PopFont();
	}

}
