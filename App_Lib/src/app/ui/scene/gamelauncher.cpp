#include "gamelauncher.h"

#include "../imguiextra.h"
#include "game/serialize.h"

#include <algorithm>
#include <utility>

namespace app::ui::scene {

	GameLauncher::GameLauncher(std::shared_ptr<TetrisController> tetrisController,
		std::function<void()> startSinglePlayer,
		std::function<void()> createNetworkGame, std::function<void()> joinNetworkGame)
		: tetrisController_{std::move(tetrisController)}
		, startSinglePlayer_{std::move(startSinglePlayer)}
		, createNetworkGame_{std::move(createNetworkGame)}
		, joinNetworkGame_{std::move(joinNetworkGame)} {
	}

	void GameLauncher::imGuiUpdate(const DeltaTime& deltaTime) {
		constexpr float contentWidth = 440.f;
		constexpr float buttonHeight = 56.f;
		constexpr float contentHeight = 330.f;
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
			startSinglePlayer_();
		}

		if (ImGui::MediumButton("Local Multiplayer", {contentWidth, buttonHeight})) {
			tetrisController_->createLocalGameRoom();
		}

		ImGui::Spacing();
		ImGui::TextUnformatted("Online multiplayer");
		const float onlineButtonWidth = (contentWidth - ImGui::GetStyle().ItemSpacing.x) / 2.f;
		const bool connected = tetrisController_->isConnectedToServer();
		ImGui::BeginDisabled(!connected);
		if (ImGui::MediumButton("Host Game", {onlineButtonWidth, buttonHeight})) {
			createNetworkGame_();
		}
		ImGui::SameLine();
		if (ImGui::MediumButton("Browse Games", {onlineButtonWidth, buttonHeight})) {
			joinNetworkGame_();
		}
		ImGui::EndDisabled();

		if (connected) {
			ImGui::SecondaryText("Connected to game server");
		} else {
			ImGui::SecondaryText("Online play unavailable - not connected to game server");
		}
		ImGui::EndGroup();
	}

}
