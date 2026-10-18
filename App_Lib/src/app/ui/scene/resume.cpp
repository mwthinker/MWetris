#include "resume.h"

#include "../imguiextra.h"

namespace app::ui::scene {

	Resume::Resume(std::shared_ptr<TetrisController> tetrisController, std::shared_ptr<game::DeviceManager> deviceManager)
		: tetrisController_{std::move(tetrisController)}
		, deviceManager_{std::move(deviceManager)} {
	}

	void Resume::imGuiUpdate(const DeltaTime& deltaTime) {
		ImVec2 buttonSize{300, 70};

		ImGui::SetCursorPos(ImVec2{(ImGui::GetWindowWidth() - buttonSize.x) / 2.0f, (ImGui::GetWindowHeight() - buttonSize.y) / 2.0f});
		if (ImGui::MediumButton("Resume Single Player F1", buttonSize)) {
			tetrisController_->createDefaultGame(deviceManager_->getDefaultDevice1());
		}
	}

}
