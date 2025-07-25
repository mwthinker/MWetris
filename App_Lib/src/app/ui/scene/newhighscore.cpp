#include "newhighscore.h"
#include "../imguiextra.h"
#include "game/serializehighscore.h"

#include <spdlog/spdlog.h>

namespace app::ui::scene {

	namespace {

		int acceptNameInput(ImGuiInputTextCallbackData* data) {
			return data->BufTextLen < 30;
		}

	}

	void NewHighScore::imGuiUpdate(const DeltaTime& deltaTime) {
		if (ImGui::IsWindowAppearing()) {
			ImGui::SetKeyboardFocusHere(0); // Set focus on input field
		}

		ImGui::PushFont(app::Configuration::getInstance().getImGuiHeaderFont(), 0.f);
		ImGui::Text("Place %d", game::getHighScorePlacement(data_.points));
		ImGui::PopFont();

		if (data_.points > 0) {
			if (ImGui::InputText("Player name", &data_.name, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CallbackAlways, acceptNameInput)
				&& data_.name.size() > 0) {
				
				save();
			}

			if (ImGui::Button("Save") && data_.name.size() > 0) {
				save();
			}
		}
	}

	void NewHighScore::save() {
		game::saveHighScore(data_.name, data_.points, data_.clearedRows, data_.level);
		data_ = {};
		closeFunction_();
	}

	void NewHighScore::switchedTo(const SceneData& sceneData) {
		try {
			data_ = dynamic_cast<const NewHighScoreData&>(sceneData);
		} catch (const std::bad_cast& exp) {
			data_ = {};
			spdlog::error("Bug, should be type NewHighScoreData: {}", exp.what());
		}
	}

}
