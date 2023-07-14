#include "customgame.h"
#include "../imguiextra.h"

#include "game/tetrisgame.h"
#include "game/devicemanager.h"

#include <array>
#include <string>

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

		void comboMode() {
			static std::array<const char*, 2> modes{"Standard Game", "Something else"};
			static int item = 0;

			ImGui::ComboScoped("##Mode", modes[item], ImGuiComboFlags_None, [&]() {
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
		}

		void comboBoardSize() {
			static std::array<const char*, 4> boards{"Default", "Small 10x12", "Large 10x26", "Crazy 50x50"};
			static int item = 0;

			ImGui::ComboScoped("##Board size", boards[item], ImGuiComboFlags_None, [&]() {
				for (int n = 0; n < boards.size(); ++n) {
					bool isSelected = (item == n);
					if (ImGui::Selectable(boards[n], isSelected)) {
						item = n;
					}
					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}
			});
		}

		int comboPlayerDevice(const std::vector<game::DevicePtr>& devices) {
			static int item = 0;

			if (item == 2) {
				//ImGui::ProgressBar(0.5f);
				ImGui::Button("Click on the device to add");
				return item;
			}

			const char* name = item < devices.size() ? devices[item]->getName() : "Add Gamepad";

			ImGui::ComboScoped("##Mode", name, ImGuiComboFlags_None, [&]() {
				for (int n = 0; n < devices.size(); ++n) {
					bool isSelected = (item == n);
					if (ImGui::Selectable(devices[n]->getName(), isSelected)) {
						item = n;
					}
					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				bool isSelected = (item == devices.size());
				if (ImGui::Selectable("Add Gamepad", isSelected)) {
					item = static_cast<int>(devices.size());
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			});
			return item;
		}

	}

	CustomGame::CustomGame(std::shared_ptr<game::TetrisGame> tetrisGame, std::shared_ptr<game::DeviceManager> deviceManager)
		: tetrisGame_{tetrisGame}
		, deviceManager_{deviceManager} {
	}

	void CustomGame::imGuiUpdate(const DeltaTime& deltaTime) {
		ImGui::PushFont(mwetris::Configuration::getInstance().getImGuiHeaderFont());
		ImGui::Text("Custom Game");
		ImGui::PopFont();

		ImGui::SeparatorText("Game Mode");
		ImGui::SetNextItemWidth(150.f);
		comboMode();

		ImGui::SeparatorText("Board Size");
		static int columns = 10;
		static int rows = 22;
		//ImGui::SliderInt("Width", &columns, 10, 50);
		//ImGui::SliderInt("Heigt", &rows, 10, 50);
		ImGui::SetNextItemWidth(150.f);
		comboBoardSize();
		
		static std::vector<game::DevicePtr> devices{
			deviceManager_->getDefaultDevice1(),
			deviceManager_->getDefaultDevice2(),
		};
		
		ImGui::SeparatorText("Players");
		static int players = 1;
		for (int i = 0; i < players; ++i) {
			ImGui::PushID(i);
			
			ImGui::SetNextItemWidth(150.f);
			comboPlayers();
			ImGui::SameLine();
			ImGui::SetNextItemWidth(150.f);
			comboPlayerDevice(devices);
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Button, sdl::color::Red);
			if (ImGui::Button("Remove##")) {
				--players;
			}
			ImGui::PopStyleColor();
			ImGui::PopID();
		}
		ImGui::PushStyleColor(ImGuiCol_Button, sdl::color::Green);
		if (ImGui::Button("Add new player", {300.f, 40.f})) {
			++players;
		}
		ImGui::PopStyleColor();

		static bool active = false;
		if (ImGui::Checkbox("AI", &active)) {
			if (active) {
				deviceManager_->searchForDevice();
			} else {
				deviceManager_->stopSearchForDevice();
			}
		}

		ImGui::PushStyleColor(ImGuiCol_Button, sdl::color::Green);
		
		float width = ImGui::GetWindowWidth() - 2 * ImGui::GetCursorPosX();
		float height = 100.f;
		float y = ImGui::GetWindowHeight() - height - ImGui::GetStyle().WindowPadding.y;
		
		ImGui::SetCursorPosY(y);
		if (ImGui::Button("Play", {width, height})) {
			//tetrisGame_->createGame()
		}

		ImGui::PopStyleColor();
	}

	void CustomGame::switchedFrom() {
		deviceManager_->stopSearchForDevice();
	}

}
