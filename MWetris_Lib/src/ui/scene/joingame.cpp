#include "joingame.h"
#include "../imguiextra.h"

#include "game/tetrisgame.h"
#include "game/devicemanager.h"

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

	JoinGame::JoinGame(std::shared_ptr<game::TetrisGame> tetrisGame, std::shared_ptr<game::DeviceManager> deviceManager)
		: tetrisGame_{tetrisGame}
		, deviceManager_{deviceManager} {

		connections_ += deviceManager->deviceConnected.connect(this, &JoinGame::deviceConnected);

		auto ais = Configuration::getInstance().getAiVector();
		for (auto& ai : ais) {
			//allAis_.emplace_back(ai.getName(), ai);
		}
	}

	void JoinGame::deviceConnected(game::DevicePtr device) {
		//allDevices_.emplace_back(device->getName(), device);
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
	}

	void JoinGame::switchedTo(const SceneData& sceneData) {
		
	}

	void JoinGame::switchedFrom() {
	}

}
