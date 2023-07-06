#include "newhighscore.h"
#include "../imguiextra.h"

namespace mwetris::ui::scene {

	namespace {

		int acceptNameInput(ImGuiInputTextCallbackData* data) {
			return data->BufTextLen < 30;
		}

	}

	void NewHighScore::imGuiUpdate(const DeltaTime& deltaTime) {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {100, 100});
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
		ImGui::PushStyleColor(ImGuiCol_PopupBg, {0, 0, 0, 0});

		ImGui::SetCursorPos({20, 230});

		if (ImGui::IsWindowAppearing()) {
			ImGui::SetKeyboardFocusHere(0);
			name_ = "Player";
		}

		ImGui::PushFont(mwetris::Configuration::getInstance().getImGuiHeaderFont());
		//ImGui::Text("Place %d", game::getPlacement(gameOver_.player->getPoints()));
		ImGui::PopFont();

		if (ImGui::InputText("Name: ", &name_, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CallbackAlways, acceptNameInput)
			&& name_.size() > 0) {

			//game::saveHighScore(name_, gameOver_.player->getPoints(), gameOver_.player->getClearedRows(), gameOver_.player->getLevel());
			ImGui::CloseCurrentPopup();
		}

		ImGui::PopStyleColor();
		ImGui::PopStyleVar(3);
	}

	void NewHighScore::switchedTo(SceneData& sceneData) {
		
	}

}
