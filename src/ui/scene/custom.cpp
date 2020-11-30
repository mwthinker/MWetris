#include "custom.h"
#include "event.h"

namespace mwetris::ui::scene {

	void Custom::imGuiUpdate(const std::chrono::high_resolution_clock::duration& deltaTime) {
		auto menuHeight = mwetris::TetrisData::getInstance().getWindowBarHeight();

		ImGui::Bar([&]() {
			ImGui::PushButtonStyle();
			if (ImGui::Button("Menu", {100.5f, menuHeight})) {
				emitEvent(Event::Menu);
			}
			ImGui::PopButtonStyle();
		});
	}

}
