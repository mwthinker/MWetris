#include "network.h"

#include "types.h"
#include "configuration.h"
#include "ui/imguiextra.h"

namespace mwetris::ui::scene {

	void Network::imGuiUpdate(const DeltaTime& deltaTime) {
		auto menuHeight = mwetris::Configuration::getInstance().getWindowBarHeight();

		ImGui::Indent(10.f);
		ImGui::Dummy({0.0f, 5.0f});

		ImGui::PushFont(mwetris::Configuration::getInstance().getImGuiDefaultFont());
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

		if (ImGui::Button("Connect")) {

		}
		ImGui::LoadingBar();

		ImGui::PopFont();
	}

}
