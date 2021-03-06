#include "custom.h"
#include "event.h"

namespace mwetris::ui::scene {

	void Custom::imGuiUpdate(const DeltaTime& deltaTime) {
		auto menuHeight = mwetris::Configuration::getInstance().getWindowBarHeight();

		ImGui::Bar([&]() {
			ImGui::PushButtonStyle();
			if (ImGui::Button("Menu", {100.5f, menuHeight})) {
				emitEvent(Event::Menu);
			}
			ImGui::PopButtonStyle();
		});
	}

}
