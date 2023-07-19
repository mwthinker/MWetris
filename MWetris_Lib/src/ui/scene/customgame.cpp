#include "customgame.h"
#include "../imguiextra.h"

#include "game/tetrisgame.h"
#include "game/devicemanager.h"

#include <array>
#include <string>
#include <map>
#include <concepts> 

namespace mwetris::ui::scene {

	namespace {

		template<typename T>
		concept HasNameMember = requires(T t) {
			{ t.name };
		};

		struct PlayerType {
			std::string name;
		};

		std::vector<PlayerType> getPlayerTypes() {
			return {
				PlayerType{
					.name = "Human",
				},
				PlayerType{
					.name = "AI",
				},
				PlayerType{
					.name = "Internet player",
				}
			};
		}

		struct GameMode {
			std::string name;
		};

		std::vector<GameMode> getGameModes() {
			return {
				GameMode{
					.name = "Standard Game",
				},
				GameMode{
					.name = "Something else",
				}
			};
		}

		struct BoardSize {
			std::string name;
			int width;
			int height;
		};

		std::vector<BoardSize> getBoardSizes() {
			return {
				BoardSize{
					.name = "Default",
					.width = 10,
					.height = 24
				},
				BoardSize{
					.name = "Small 10x12",
					.width = 10,
					.height = 12
				},
				BoardSize{
					.name = "Large 10x26",
					.width = 10,
					.height = 26
				},
				BoardSize{
					.name = "Crazy 50x50",
					.width = 50,
					.height = 50
				}
			};
		}

		std::vector<DeviceType> getDeviceTypes(const std::vector<game::DevicePtr>& devices, const char* addGamePad) {
			std::vector<DeviceType> types;

			for (const auto& device : devices) {
				types.emplace_back(device->getName(), device);
			}
			types.emplace_back(addGamePad, nullptr);
			return types;
		}

		template <typename Type> requires HasNameMember<Type>
		const Type& comboType(const char* label, const std::vector<Type>& boards) {
			static std::map<const char*, int> indexByLabel;
			auto& item = indexByLabel[label];

			ImGui::ComboScoped(label, boards[item].name.c_str(), ImGuiComboFlags_None, [&]() {
				for (int n = 0; n < boards.size(); ++n) {
					bool isSelected = (item == n);
					if (ImGui::Selectable(boards[n].name.c_str(), isSelected)) {
						item = n;
					}
					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}
			});

			return boards[item];
		}

	}

	CustomGame::CustomGame(std::shared_ptr<game::TetrisGame> tetrisGame, std::shared_ptr<game::DeviceManager> deviceManager)
		: tetrisGame_{tetrisGame}
		, deviceManager_{deviceManager} {

		connections_ += deviceManager->deviceConnected.connect(this, &CustomGame::deviceConnected);
	}

	void CustomGame::deviceConnected(game::DevicePtr device) {
		allDevices_.emplace_back(device->getName(), device);
	}

	void CustomGame::imGuiUpdate(const DeltaTime& deltaTime) {
		ImGui::PushFont(mwetris::Configuration::getInstance().getImGuiHeaderFont());
		ImGui::Text("Custom Game");
		ImGui::PopFont();

		ImGui::SeparatorText("Game Mode");
		ImGui::SetNextItemWidth(150.f);

		static auto gameModes = getGameModes();
		comboType<GameMode>("##Game Mode", gameModes);

		ImGui::SeparatorText("Board Size");
		ImGui::SetNextItemWidth(150.f);

		static auto boardSizes = getBoardSizes();
		const auto& boardSize = comboType<BoardSize>("##Board size", boardSizes);

		static auto playerTypes = getPlayerTypes();
		
		ImGui::SeparatorText("Players");
		static int players = 1;
		for (int i = 0; i < players; ++i) {
			ImGui::PushID(i);
			
			ImGui::SetNextItemWidth(150.f);
			comboType<PlayerType>("##Player Type", playerTypes);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(150.f);
			//comboPlayerDevice(devices);
			const auto& deviceType = comboType<DeviceType>("##Players", allDevices_);
			if (i != 0) {
				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Button, sdl::color::Red);
				if (ImGui::Button("Remove##")) {
					--players;
				}
				ImGui::PopStyleColor();
			}
			ImGui::PopID();
		}
		ImGui::PushStyleColor(ImGuiCol_Button, sdl::color::Green);
		if (ImGui::Button("Add new player", {300.f, 40.f})) {
			++players;
		}
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_Button, sdl::color::Green);
		
		float width = ImGui::GetWindowWidth() - 2 * ImGui::GetCursorPosX();
		float height = 100.f;
		float y = ImGui::GetWindowHeight() - height - ImGui::GetStyle().WindowPadding.y;
		
		ImGui::SetCursorPosY(y);
		if (ImGui::Button("Play", {width, height})) {
			tetrisGame_->createGame(boardSize.width, boardSize.height, {deviceManager_->getDefaultDevice1()});
			ImGui::CloseCurrentPopup();
		}
		
		ImGui::PopStyleColor();
	}

	void CustomGame::switchedTo(const SceneData& sceneData) {
		allDevices_ = getDeviceTypes(deviceManager_->getAllDevicesAvailable(), "Add Game Pad");
	}

	void CustomGame::switchedFrom() {
	}

}
