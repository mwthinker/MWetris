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
		const bool focusName = ImGui::IsWindowAppearing();

		ImGui::PushFont(app::Configuration::getInstance().getImGuiSemiBoldFont(), 0.f);
		ImGui::Text("Place %d", game::getHighScorePlacement(data_.points));
		ImGui::PopFont();

		if (data_.points > 0) {
			bool submitted = false;
			ImGui::BeginModalBody("High score content");
			ImGui::FormTable("High score form", [&]() {
				ImGui::FormRow("Player name", [&]() {
					if (focusName) {
						ImGui::SetKeyboardFocusHere();
					}
					submitted = ImGui::InputText("##Value", &data_.name,
						ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CallbackAlways,
						acceptNameInput);
				});
			});
			ImGui::EndModalBody();

			ImGui::AlignNextItemRight(100.f);
			if ((ImGui::ConfirmationButton("Save", {100.f, 0.f}) || submitted) && !data_.name.empty()) {
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
