#include "creategameroom.h"
#include "addplayer.h"
#include "tetriscontroller.h"
#include "util.h"

#include "game/devicemanager.h"
#include "game/playerslot.h"
#include "game/tetrisgame.h"
#include "network/network.h"
#include "ui/imguiextra.h"
#include "tetriscontroller.h"

#include <array>
#include <string>
#include <map>
#include <concepts>
#include <variant>
#include <memory>

namespace mwetris::ui::scene {

	namespace {

		int acceptNameInput(ImGuiInputTextCallbackData* data) {
			return data->BufTextLen < 30;
		}

		bool InputText(const char* label, std::string* text) {
			return ImGui::InputText(label, text, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CallbackAlways, acceptNameInput);
		}

	}

	CreateGameRoom::CreateGameRoom(std::shared_ptr<mwetris::TetrisController> tetrisController)
		: tetrisController_{tetrisController} {

		connections_ += tetrisController_->tetrisEvent.connect([this](const TetrisEvent& event) {
			if (auto gameRoomListEvent = std::get_if<network::GameRoomListEvent>(&event)) {
				//gameRooms_ = gameRoomListEvent->gameRooms;
			}
		});
	}

	void CreateGameRoom::imGuiUpdate(const DeltaTime& deltaTime) {
		ImGui::PushFont(mwetris::Configuration::getInstance().getImGuiHeaderFont());
		ImGui::Text("Create Game Room");
		ImGui::PopFont();

		ImGui::Text("Name");
		if (InputText("##ServerName", &serverName_)) {
		}

		ImGui::Checkbox("Public", &public_);

		ImGui::PopupContextItem([&]() {
			if (ImGui::MenuItem("Paste", "CTRL+V")) {
				serverName_.assign(ImGui::GetClipboardText());
				ImGui::CloseCurrentPopup();
			}
		});

		bool validName = serverName_.size() > 5 && serverName_.size() < 30;
		ImGui::BeginDisabled(!validName);
		if (ImGui::ConfirmationButton("Create")) { // TODO! Handle lag and show a spinner.
			tetrisController_->createNetworkGameRoom(serverName_, public_);
		}
		ImGui::EndDisabled();
	}

	void CreateGameRoom::switchedTo(const SceneData& sceneData) {
		serverName_ = "";
		public_ = false;
	}

	void CreateGameRoom::switchedFrom() {
	}

}
