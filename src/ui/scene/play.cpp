#include "play.h"

namespace tetris::ui::scene {

	void Play::draw(const std::chrono::high_resolution_clock::duration& deltaTime) {

	}

	void Play::imGuiUpdate(const std::chrono::high_resolution_clock::duration& deltaTime) {
		auto menuHeight = tetris::TetrisData::getInstance().getWindowBarHeight();

		ImGui::Bar([&]() {
			ImGui::PushButtonStyle();

			if (ImGui::Button("Menu", {100.5f, menuHeight})) {
				//changePage(Page::MENU);
			}
			ImGui::SameLine();
			//if (ImGui::Button("Restart", {120.5f, menuHeight_})) {
				//game_.restartGame();
			//}

			ImGui::SameLine();
			//if (ImGui::ManButton("Human", nbrHumans_, static_cast<int>(devices_.size()), noManTexture_.getTextureView(), manTexture_.getTextureView(), {menuHeight, menuHeight})) {
				//game_.createGame(getCurrentDevices());
			//}
			ImGui::SameLine();
			//if (ImGui::ManButton("Ai", nbrAis_, static_cast<int>(activeAis_.size()), noManTexture_.getTextureView(), aiTexture_.getTextureView(), {menuHeight, menuHeight})) {
				//game_.createGame(getCurrentDevices());
			//}
			ImGui::PopButtonStyle();
		});
	}

}
