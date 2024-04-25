#include "joingame.h"
#include "../imguiextra.h"

#include "game/tetrisgame.h"
#include "network/network.h"

#include <array>
#include <string>
#include <map>
#include <concepts> 

namespace mwetris::ui::scene {

	namespace {

		int acceptNameInput(ImGuiInputTextCallbackData* data) {
			return data->BufTextLen < 30;
		}

	}

	JoinGame::JoinGame(std::shared_ptr<game::TetrisGame> game, std::shared_ptr<network::Network> network)
		: network_{network}
		, game_{game} {

		connections_ += network_->addPlayerSlotListener([this](game::PlayerSlot, int) {
			connected_ = true;
		});
	}

	void JoinGame::imGuiUpdate(const DeltaTime& deltaTime) {
		ImGui::PushFont(mwetris::Configuration::getInstance().getImGuiHeaderFont());
		ImGui::Text("Custom Game");
		ImGui::PopFont();

		ImGui::Text("Server Id");
		if (ImGui::InputText("##ServerId", &serverId_, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CallbackAlways, acceptNameInput)) {
			
		}

		ImGui::PopupContextItem([&]() {
			if (ImGui::MenuItem("Paste", "CTRL+V")) {
				serverId_.assign(ImGui::GetClipboardText());
				ImGui::CloseCurrentPopup();
			}
		});

		if (ImGui::ConfirmationButton("Connect")) {
			network_->connectToGame(serverId_);
		}
	}

	void JoinGame::switchedTo(const SceneData& sceneData) {
		connected_ = false;
	}

	void JoinGame::switchedFrom() {
	}

}
