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
		const float scale = ImGui::GetUiScale();
		ImGui::PushFont(app::Configuration::getInstance().getImGuiSemiBoldFont(), 0.f);
		ImGui::Text("Browse Game");
		ImGui::PopFont();

		ImGui::FormTable("Join game room form", [&]() {
			ImGui::FormRow("Server ID", [&]() {
				InputText("##Value", &serverId_);
				ImGui::PopupContextItem([&]() {
					if (ImGui::MenuItem("Paste", "CTRL+V")) {
						serverId_.assign(ImGui::GetClipboardText());
						ImGui::CloseCurrentPopup();
					}
				});
			});
		}, 160.f * scale);

		const float buttonWidth = 100.f * scale;
		const float actionWidth = buttonWidth * 2.f + ImGui::GetStyle().ItemSpacing.x;
		ImGui::AlignNextItemRight(actionWidth);
		if (ImGui::MediumButton("Refresh", {buttonWidth, 30.f * scale})) {
			tetrisController_->refreshGameRoomList();
		}
		ImGui::SameLine();
		if (ImGui::ConfirmationButton("Connect", {buttonWidth, 30.f * scale})) { // TODO! Handle lag and show a spinner.
			tetrisController_->joinGameRoom(serverId_);
		}

		ImGui::FormTable("Game room filter", [&]() {
			ImGui::FormRow("Filter", [&]() { InputText("##Value", &filter_); });
		}, 160.f * scale);

		ImGui::Table("Game rooms", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp, {0.f, 0.f}, [&]() {
			ImGui::TableSetupColumn("Id", ImGuiTableColumnFlags_WidthStretch, 1.f);
			ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 1.f);
			ImGui::TableSetupColumn("Players / Max", ImGuiTableColumnFlags_WidthFixed, 120.f * scale);
			ImGui::TableSetupColumn("Connect", ImGuiTableColumnFlags_WidthFixed, 110.f * scale);
			ImGui::TableHeadersRow();

			for (const auto& gameRoom : gameRooms_) {
				ImGui::TableNextRow();
				ImGui::TableNextColumn(); ImGui::Text("%s", gameRoom.id.c_str());
				ImGui::TableNextColumn(); ImGui::Text("%s", gameRoom.name.c_str());
				ImGui::TableNextColumn(); ImGui::Text("%i / %i", gameRoom.playerCount, gameRoom.maxPlayerCount);

				ImGui::TableNextColumn();
				if (ImGui::MediumButton("Connect")) {
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
