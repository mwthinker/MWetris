#include "leavegame.h"

#include "../imguiextra.h"
#include "configuration.h"

#include <utility>

namespace app::ui::scene {

	LeaveGame::LeaveGame(std::shared_ptr<TetrisController> tetrisController, std::function<void()> quitApplication)
		: tetrisController_{std::move(tetrisController)}
		, quitApplication_{std::move(quitApplication)} {
	}

	void LeaveGame::imGuiUpdate(const DeltaTime& deltaTime) {
		const bool networkGame = tetrisController_->isNetworkGame();
		const bool singlePlayerGame = tetrisController_->isDefaultGame();

		ImGui::PushFont(app::Configuration::getInstance().getImGuiSemiBoldFont(), 0.f);
		ImGui::TextUnformatted(quitAfterConfirmation_ ? "Quit MWetris?"
			: networkGame ? "Leave online game?"
			: singlePlayerGame ? "Leave single-player game?"
			: "Leave local game?");
		ImGui::PopFont();
		ImGui::BeginModalBody("Leave game content");
		ImGui::Spacing();

		if (networkGame) {
			ImGui::TextWrapped(quitAfterConfirmation_
				? "MWetris will close and you will disconnect from the room and the other players."
				: "You will disconnect from the room and the other players.");
		} else if (singlePlayerGame) {
			ImGui::TextWrapped(quitAfterConfirmation_
				? "MWetris will close. Your current progress is saved, so you can continue later."
				: "Your current progress is saved, so you can continue later.");
		} else {
			ImGui::TextWrapped(quitAfterConfirmation_
				? "MWetris will close and end the game for all local players. The current game cannot be continued."
				: "This will end the game for all local players. The current game cannot be continued.");
		}
		ImGui::EndModalBody();

		const float buttonWidth = 150.f;
		const float actionWidth = buttonWidth * 2.f + ImGui::GetStyle().ItemSpacing.x;
		ImGui::AlignNextItemRight(actionWidth);

		ImGui::BeginDisabled(leaving_);
		if (ImGui::MediumButton("Keep Playing", {buttonWidth, 0.f})) {
			close();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		const char* destructiveLabel = quitAfterConfirmation_ ? "Quit MWetris" : leaving_ ? "Leaving..." : "Leave Game";
		if (ImGui::AbortButton(destructiveLabel, {buttonWidth, 0.f})) {
			leaving_ = true;
			if (quitAfterConfirmation_) {
				quitApplication_();
			} else {
				tetrisController_->leaveGameRoom();
			}
		}
		ImGui::EndDisabled();
	}

	void LeaveGame::switchedTo(const SceneData& sceneData) {
		leaving_ = false;
		if (const auto* data = dynamic_cast<const LeaveGameData*>(&sceneData)) {
			quitAfterConfirmation_ = data->quitApplication;
		} else {
			quitAfterConfirmation_ = false;
		}
	}

}
