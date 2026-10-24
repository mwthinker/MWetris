#include "creategameroom.h"
#include "addplayer.h"
#include "tetriscontroller.h"

#include "game/devicemanager.h"
#include "game/playerslot.h"
#include "game/tetrisgame.h"
#include "cnetwork/network.h"
#include "ui/imguiextra.h"
#include "util/auxiliary.h"

#include <array>
#include <string>
#include <map>
#include <concepts>
#include <variant>
#include <memory>

namespace app::ui::scene {

	namespace {

		int acceptNameInput(ImGuiInputTextCallbackData* data) {
			return data->BufTextLen < 30;
		}

		bool InputText(const char* label, std::string* text) {
			return ImGui::InputText(label, text, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CallbackAlways, acceptNameInput);
		}

	}

	CreateGameRoom::CreateGameRoom(std::shared_ptr<app::TetrisController> tetrisController)
		: tetrisController_{tetrisController} {

		connections_ += tetrisController_->tetrisEvent.connect([this](const TetrisEvent& event) {
			if (auto gameRoomListEvent = std::get_if<cnetwork::GameRoomListEvent>(&event)) {
				//gameRooms_ = gameRoomListEvent->gameRooms;
			}
		});
	}

	void CreateGameRoom::imGuiUpdate(const DeltaTime& deltaTime) {
		ImGui::PushFont(app::Configuration::getInstance().getImGuiSemiBoldFont(), 0.f);
		ImGui::Text("Create Game Room");
		ImGui::PopFont();

		ImGui::BeginModalBody("Create game room content");
		ImGui::FormTable("Create game room form", [&]() {
			ImGui::FormRow("Name", [&]() {
				InputText("##Value", &serverName_);
				ImGui::PopupContextItem([&]() {
					if (ImGui::MenuItem("Paste", "CTRL+V")) {
						serverName_.assign(ImGui::GetClipboardText());
						ImGui::CloseCurrentPopup();
					}
				});
			});
			ImGui::FormRow("Public", [&]() {
				ImGui::Checkbox("##Value", &public_);
			});
		});
		ImGui::EndModalBody();

		bool validName = serverName_.size() > 5 && serverName_.size() < 30;
		ImGui::AlignNextItemRight(100.f);
		ImGui::BeginDisabled(!validName);
		if (ImGui::ConfirmationButton("Create", {100.f, 0.f})) { // TODO! Handle lag and show a spinner.
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
