#include "controls.h"

#include "ui/controlspresenter.h"
#include "configuration.h"

#include <utility>

namespace app::ui::scene {

	Controls::Controls(std::shared_ptr<ControlsPresenter> controlsPresenter)
		: controlsPresenter_{std::move(controlsPresenter)} {
	}

	void Controls::imGuiUpdate(const DeltaTime&) {
		ImGui::PushFont(Configuration::getInstance().getImGuiSemiBoldFont(), 0.f);
		ImGui::TextUnformatted("Controls");
		ImGui::PopFont();
		ImGui::Separator();
		controlsPresenter_->drawMenu();
	}

}
