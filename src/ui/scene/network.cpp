#include "network.h"
#include "event.h"

namespace tetris::ui::scene {

	void Network::imGuiUpdate(const std::chrono::high_resolution_clock::duration& deltaTime) {
		auto menuHeight = tetris::TetrisData::getInstance().getWindowBarHeight();

		ImGui::Bar([&]() {
			ImGui::PushButtonStyle();
			if (ImGui::Button("Menu", {100.5f, menuHeight})) {
				emitEvent<Event>(Event::Menu);
			}
			ImGui::PopButtonStyle();
		});

		ImGui::Indent(10.f);
		ImGui::Dummy({0.0f, 5.0f});

		ImGui::PushFont(tetris::TetrisData::getInstance().getImGuiDefaultFont());
		static int radioNbr = 0;
		ImGui::RadioButton("Server", &radioNbr, 0);
		ImGui::RadioButton("Client", &radioNbr, 1);

		ImGui::PushItemWidth(150);
		if (radioNbr == 0) {
			static char ipField[30] = "";
			ImGui::InputText("The ip-number for the server", ipField, 30);
			if (ImGui::BeginPopupContextItem("item context menu", ImGuiMouseButton_Right)) {
				if (ImGui::Selectable("Copy")) {
					ImGui::SetClipboardText(ipField);
				}
				if (ImGui::Selectable("Paste")) {
					std::strcpy(ipField, ImGui::GetClipboardText());
				}
				ImGui::EndPopup();
			}
		}
		static int port = 5012;
		ImGui::InputInt("The port-number for the server", &port);

		ImGui::PopItemWidth();
		ImGui::Dummy({0.f, 5.f});

		if (ImGui::Button("Connect", {100.5f, menuHeight})) {

		}
		ImGui::LoadingBar();

		ImGui::PopFont();
	}

}
