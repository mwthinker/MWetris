#include "addplayer.h"
#include "../imguiextra.h"

#include "game/tetrisgame.h"
#include "game/devicemanager.h"

#include <array>
#include <string>
#include <map>
#include <concepts> 

namespace mwetris::ui::scene {

	namespace {

		enum class Player{
			Human,
			Ai
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

		std::vector<game::Human> getDeviceTypes(const std::vector<game::DevicePtr>& devices) {
			std::vector<game::Human> types;

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

	AddPlayer::AddPlayer(std::function<void(AddPlayer&)> addCallback, std::shared_ptr<game::DeviceManager> deviceManager)
		: addCallback_{addCallback}
		, deviceManager_{deviceManager} {

		connections_ += deviceManager->deviceConnected.connect(this, &AddPlayer::deviceConnected);

		auto ais = Configuration::getInstance().getAiVector();
		for (auto& ai : ais) {
			allAis_.emplace_back(ai.getName(), ai);
		}
	}

	void AddPlayer::deviceConnected(game::DevicePtr device) {
		allDevices_.emplace_back(device->getName(), device);
	}

	void AddPlayer::imGuiUpdate(const DeltaTime& deltaTime) {
		ImGui::PushFont(mwetris::Configuration::getInstance().getImGuiHeaderFont());
		ImGui::Text("Add Player");
		ImGui::PopFont();

		static auto playerTypes = getPlayerTypes();
		ImGui::SetNextItemWidth(150.f);
		auto playerType = ImGui::ComboUniqueType<PlayerType>("##Player Type", playerTypes);
		
		if (playerType.player == Player::Human) {
			ImGui::SameLine();
			ImGui::SetNextItemWidth(150.f);
			auto result = ImGui::ComboUniqueType<game::Human>("##Players", allDevices_);
			result.name = playerName_;
			player_ = result;
		} else if (playerType.player == Player::Ai) {
			ImGui::SameLine();
			ImGui::SetNextItemWidth(150.f);
			auto result = ImGui::ComboUniqueType<game::Ai>("##Players", allAis_);
			result.name = playerName_;
			player_ = result;
		}

		ImGui::SameLine();
		ImGui::SetNextItemWidth(100.f);
		ImGui::InputText("##PlayerName", &playerName_);

		float width = ImGui::GetWindowWidth() - 2 * ImGui::GetCursorPosX();
		float height = 100.f;
		float y = ImGui::GetWindowHeight() - height - ImGui::GetStyle().WindowPadding.y;

		ImGui::SetCursorPosY(y);
		if (ImGui::ConfirmationButton("Add", {width, height})) {
			addCallback_(*this);
			ImGui::CloseCurrentPopup();
		}
	}

	void AddPlayer::switchedTo(const SceneData& sceneData) {
		try {
			data_ = dynamic_cast<const AddPlayerData&>(sceneData);
		} catch (const std::bad_cast& exp) {
			data_ = {};
			spdlog::error("Bug, should be type NewHighScoreData: {}", exp.what());
		}
		playerName_ = "Player 1";
		player_ = game::Human{
			.name = playerName_,
			.device = deviceManager_->getDefaultDevice1()
		};
		allDevices_ = getDeviceTypes(deviceManager_->getAllDevicesAvailable());
	}

	void AddPlayer::switchedFrom() {
	}

}
