#include "gamelauncher.h"

#include "../imguiextra.h"
#include "game/serialize.h"

#include <algorithm>
#include <utility>

namespace app::ui::scene {

	GameLauncher::GameLauncher(std::shared_ptr<TetrisController> tetrisController, const GameLauncherActions& actions)
		: tetrisController_{std::move(tetrisController)}
		, actions_{actions} {
	}

	void GameLauncher::imGuiUpdate(const DeltaTime& deltaTime) {
		const float scale = ImGui::GetUiScale();
		const float contentWidth = 440.f * scale;
		const float buttonHeight = 56.f * scale;
		const float contentHeight = 480.f * scale;
		const float left = std::max(ImGui::GetStyle().WindowPadding.x, (ImGui::GetWindowWidth() - contentWidth) / 2.f);
		const float top = std::max(ImGui::GetCursorPosY(), (ImGui::GetWindowHeight() - contentHeight) / 2.f);

		ImGui::SetCursorPos({left, top});
		ImGui::BeginGroup();
		ImGui::PushFont(app::Configuration::getInstance().getImGuiSemiBoldFont(), 0.f);
		ImGui::TextUnformatted("Choose how to play");
		ImGui::PopFont();
		ImGui::SecondaryText("Continue on your own or start a game with others.");
		ImGui::Spacing();

		const char* singlePlayerLabel = game::hasSavedGame() ? "Resume Single Player   F1" : "Start Single Player   F1";
		if (ImGui::MediumButton(singlePlayerLabel, {contentWidth, buttonHeight})) {
			actions_.startSinglePlayer();
		}

		if (ImGui::MediumButton("Local Multiplayer", {contentWidth, buttonHeight})) {
			tetrisController_->createLocalGameRoom();
		}
		if (ImGui::MediumButton("High Scores", {contentWidth, buttonHeight})) {
			actions_.showHighScores();
		}

		ImGui::Spacing();
		ImGui::TextUnformatted("Online multiplayer");
		const float onlineButtonWidth = (contentWidth - ImGui::GetStyle().ItemSpacing.x) / 2.f;
		const bool connected = tetrisController_->isConnectedToServer();
		ImGui::BeginDisabled(!connected);
		if (ImGui::MediumButton("Host Game", {onlineButtonWidth, buttonHeight})) {
			actions_.createNetworkGame();
		}
		ImGui::SameLine();
		if (ImGui::MediumButton("Browse Games", {onlineButtonWidth, buttonHeight})) {
			actions_.joinNetworkGame();
		}
		ImGui::EndDisabled();

		if (connected) {
			ImGui::SecondaryText("Connected to game server");
		} else {
			ImGui::SecondaryText("Online play unavailable - not connected to game server");
		}

		ImGui::Spacing();
		if (ImGui::MediumButton("Preferences", {contentWidth, buttonHeight})) {
			actions_.showPreferences();
		}
		ImGui::Spacing();
		if (ImGui::MediumButton("Quit", {contentWidth, buttonHeight})) {
			actions_.quit();
		}
		ImGui::EndGroup();
	}

}
