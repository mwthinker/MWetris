#include "customgame.h"
#include "../imguiextra.h"

#include <array>

namespace mwetris::ui::scene {

	namespace {

		void comboPlayers() {
			static std::array<const char*, 3> types{"Human", "AI", "Internet player"};
			static int item = 0;

			ImGui::ComboScoped("", types[item], ImGuiComboFlags_None, [&]() {
				for (int n = 0; n < types.size(); ++n) {
					ImGui::PushID(n);
					bool isSelected = (item == n);
					if (ImGui::Selectable(types[n], isSelected)) {
						item = n;
					}
					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
					ImGui::PopID();
				}
			});
		}

	}

	void CustomGame::imGuiUpdate(const DeltaTime& deltaTime) {
		ImGui::PushFont(mwetris::Configuration::getInstance().getImGuiHeaderFont());
		ImGui::Text("Custom Game");
		ImGui::PopFont();

		static std::array<const char*, 2> modes{"Standard Game", "Something else"};
		static int item = 0;

		ImGui::ComboScoped("Mode", modes[item], ImGuiComboFlags_None, [&]() {
			for (int n = 0; n < modes.size(); ++n) {
				bool isSelected = (item == n);
				if (ImGui::Selectable(modes[n], isSelected)) {
					item = n;
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
		});
		ImGui::SeparatorText("Board size");
		static int columns = 10;
		static int rows = 22;
		ImGui::SliderInt("Width", &columns, 10, 50);
		ImGui::SliderInt("Heigt", &rows, 10, 50);
		ImGui::SeparatorText("Players");
		static int players = 1;
		for (int i = 0; i < players; ++i) {
			ImGui::PushID(i);
			if (ImGui::Button("-##")) {
				--players;
			}
			ImGui::SameLine();
			ImGui::Text("Player");
			ImGui::SameLine();
			comboPlayers();
			ImGui::PopID();
		}
		if (ImGui::Button("+")) {
			++players;
		}
		ImGui::PushStyleColor(ImGuiCol_Button, sdl::color::Green);
		
		float width = ImGui::GetWindowWidth() - 2 * ImGui::GetCursorPosX();
		float height = 100.f;
		float y = ImGui::GetWindowHeight() - height - ImGui::GetStyle().WindowPadding.y;
		
		ImGui::SetCursorPosY(y);
		ImGui::Button("Play", {width, height});
		ImGui::PopStyleColor();
	}

}
