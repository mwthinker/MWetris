#include "joingameroom.h"
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

	JoinGameRoom::JoinGameRoom(std::shared_ptr<TetrisController> tetrisController)
		: tetrisController_{tetrisController} {

		connections_ += tetrisController_->tetrisEvent.connect([this](const TetrisEvent& event) {
			if (auto gameRoomListEvent = std::get_if<cnetwork::GameRoomListEvent>(&event)) {
				gameRooms_ = gameRoomListEvent->gameRooms;
			}
		});
	}

	void JoinGameRoom::imGuiUpdate(const DeltaTime& deltaTime) {
		ImGui::PushFont(app::Configuration::getInstance().getImGuiHeaderFont());
		ImGui::Text("Custom Game");
		ImGui::PopFont();

		ImGui::Text("Server Id");
		if (InputText("##ServerId", &serverId_)) {
		}

		ImGui::PopupContextItem([&]() {
			if (ImGui::MenuItem("Paste", "CTRL+V")) {
				serverId_.assign(ImGui::GetClipboardText());
				ImGui::CloseCurrentPopup();
			}
		});

		if (ImGui::ConfirmationButton("Connect")) { // TODO! Handle lag and show a spinner.
			tetrisController_->joinGameRoom(serverId_);
		}

		if (ImGui::Button("Refresh")) {
			tetrisController_->refreshGameRoomList();
		}

		if (InputText("Filter", &filter_)) {
		}

		ImGui::Table("Game rooms", 4, ImGuiTableFlags_Borders, {500, 0}, [&]() {
			ImGui::TableSetupColumn("Id");// , ImGuiTableColumnFlags_WidthFixed, 70);
			ImGui::TableSetupColumn("Name");
			ImGui::TableSetupColumn("Players / Max");
			ImGui::TableSetupColumn("Connect");
			ImGui::TableHeadersRow();

			for (const auto& gameRoom : gameRooms_) {
				ImGui::TableNextRow();
				ImGui::TableNextColumn(); ImGui::Text("%s", gameRoom.id.c_str());
				ImGui::TableNextColumn(); ImGui::Text("%s", gameRoom.name.c_str());
				ImGui::TableNextColumn(); ImGui::Text("%i / %i", gameRoom.playerCount, gameRoom.maxPlayerCount);

				ImGui::TableNextColumn();
				if (ImGui::Button("Connect")) {
					tetrisController_->joinGameRoom(gameRoom.id.c_str());
				}
			}
		});
	}

	void JoinGameRoom::switchedTo(const SceneData& sceneData) {
		gameRooms_.clear();
		serverId_ = "";
		filter_ = "";
		tetrisController_->refreshGameRoomList();
	}

	void JoinGameRoom::switchedFrom() {
	}

}
