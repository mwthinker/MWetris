#include "creategame.h"
#include "../imguiextra.h"

#include "game/tetrisgame.h"
#include "game/devicemanager.h"

#include <array>
#include <string>
#include <map>
#include <concepts> 

namespace mwetris::ui::scene {

	namespace {

		enum class Player {
			Human,
			Ai,
			InternetPlayer
		};

		struct PlayerType {
			Player player;
			std::string name;
		};

		std::vector<PlayerType> getPlayerTypes() {
			return {
				PlayerType{
					.player = Player::Human,
					.name = "Human"
				},
				PlayerType{
					.player = Player::Ai,
					.name = "AI"
				},
				PlayerType{
					.player = Player::InternetPlayer,
					.name = "Internet player"
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

		struct XX {};

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

		std::vector<DeviceType> getDeviceTypes(const std::vector<game::DevicePtr>& devices) {
			std::vector<DeviceType> types;

			for (const auto& device : devices) {
				types.emplace_back(device->getName(), device);
			}
			return types;
		}
		
		std::vector<game::Human> extractHumans(const std::vector<std::string>& names, const std::vector<std::variant<game::DevicePtr, tetris::Ai>>& players) {
			std::vector<game::Human> humans;
			for (int i = 0; i < players.size(); ++i) {
				const auto& player = players[i];

				if (std::holds_alternative<game::DevicePtr>(player)) {
					const auto& device = std::get<game::DevicePtr>(player);
					humans.push_back(game::Human{.name = names[i], .device = device});
				}
			}
			return humans;
		}

		std::vector<game::Ai> extractAis(const std::vector<std::string>& names, const std::vector<std::variant<game::DevicePtr, tetris::Ai>>& players) {
			std::vector<game::Ai> ais;
			for (int i = 0; i < players.size(); ++i) {
				const auto& player = players[i];

				if (std::holds_alternative<tetris::Ai>(player)) {
					const auto& ai = std::get<tetris::Ai>(player);
					ais.push_back(game::Ai{.name = names[i], .ai = ai});
				}
			}
			return ais;
		}

	}

	CreateGame::CreateGame(std::shared_ptr<game::TetrisGame> tetrisGame, std::shared_ptr<game::DeviceManager> deviceManager)
		: tetrisGame_{tetrisGame}
		, deviceManager_{deviceManager} {

		connections_ += deviceManager->deviceConnected.connect(this, &CreateGame::deviceConnected);

		auto ais = Configuration::getInstance().getAiVector();
		for (auto& ai : ais) {
			allAis_.emplace_back(ai.getName(), ai);
		}
	}

	void CreateGame::deviceConnected(game::DevicePtr device) {
		allDevices_.emplace_back(device->getName(), device);
	}

	void CreateGame::imGuiUpdate(const DeltaTime& deltaTime) {
		ImGui::PushFont(mwetris::Configuration::getInstance().getImGuiHeaderFont());
		ImGui::Text("Create Game");
		ImGui::PopFont();

		ImGui::SeparatorText("Game Mode");
		ImGui::SetNextItemWidth(150.f);

		static auto gameModes = getGameModes();
		ImGui::ComboUniqueType<GameMode>("##Game Mode", gameModes);

		ImGui::SeparatorText("Board Size");
		ImGui::SetNextItemWidth(150.f);

		static auto boardSizes = getBoardSizes();
		const auto& boardSize = ImGui::ComboUniqueType<BoardSize>("##Board size", boardSizes);

		static auto playerTypes = getPlayerTypes();

		ImGui::SeparatorText("Players");

		for (int i = 0; i < playerNames_.size(); ++i) {
			ImGui::PushID(i);

			ImGui::SetNextItemWidth(150.f);
			auto playerType = ImGui::ComboUniqueType<PlayerType>("##Player Type", playerTypes, i);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(150.f);

			if (playerType.player == Player::Human) {
				const auto& deviceType = ImGui::ComboUniqueType<DeviceType>("##Players", allDevices_, i);
				players_[i] = deviceType.device;
			} else if (playerType.player == Player::Ai) {
				const auto& aiType = ImGui::ComboUniqueType<AiType>("##Players", allAis_, i);
				players_[i] = aiType.ai;
			}

			ImGui::SameLine();
			ImGui::SetNextItemWidth(100.f);
			ImGui::InputText("", &playerNames_[i]);

			if (i != 0) {
				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Button, sdl::color::Red);
				if (ImGui::Button("Remove##")) {
					playerNames_.pop_back();
					allAis_.pop_back();
					players_.pop_back();
				}
				ImGui::PopStyleColor();
			}
			ImGui::PopID();
		}

		//ImGui::BeginDisabled();
		ImGui::PushStyleColor(ImGuiCol_Button, sdl::color::Green);
		if (ImGui::Button("Add new player", {300.f, 40.f})) {
			playerNames_.push_back(fmt::format("Player {}", playerNames_.size() + 1));
			players_.push_back(deviceManager_->getDefaultDevice1());
		}
		ImGui::PopStyleColor();
		//ImGui::EndDisabled();

		ImGui::PushStyleColor(ImGuiCol_Button, sdl::color::Green);

		float width = ImGui::GetWindowWidth() - 2 * ImGui::GetCursorPosX();
		float height = 100.f;
		float y = ImGui::GetWindowHeight() - height - ImGui::GetStyle().WindowPadding.y;

		ImGui::SetCursorPosY(y);
		if (ImGui::Button("Play", {width, height})) {
			tetrisGame_->createGame(boardSize.width, boardSize.height,
				extractHumans(playerNames_, players_),
				extractAis(playerNames_, players_)
			);
			ImGui::CloseCurrentPopup();
		}
		ImGui::PopStyleColor();
	}

	void CreateGame::switchedTo(const SceneData& sceneData) {
		playerNames_.clear();
		playerNames_.push_back("Player 1");
		players_.clear();
		players_.push_back(deviceManager_->getDefaultDevice1());
		allDevices_ = getDeviceTypes(deviceManager_->getAllDevicesAvailable());
	}

	void CreateGame::switchedFrom() {
	}

}
