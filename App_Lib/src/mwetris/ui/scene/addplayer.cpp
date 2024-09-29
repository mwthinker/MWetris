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

		std::vector<game::Human> getDeviceTypes(const std::vector<game::DevicePtr>& devices, const std::vector<game::DevicePtr>& excludes) {
			std::vector<game::Human> types;

			for (const auto& device : devices) {
				if (std::find(excludes.begin(), excludes.end(), device) != excludes.end()) continue;

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
		, deviceManager_{deviceManager}
		, gameModesCombo_{getGameModes()}
		, playerTypesCombo_{getPlayerTypes(allDevices_.empty())}
		, humansCombo_{allDevices_}
		, aisCombo_{allAis_} {

		connections_ += deviceManager->deviceConnected.connect(this, &AddPlayer::deviceConnected);

		auto ais = Configuration::getInstance().getAiVector();
		for (auto& ai : ais) {
			allAis_.emplace_back(ai.name, ai.ai);
		}
	}

	void AddPlayer::deviceConnected(game::DevicePtr device) {
		allDevices_.emplace_back(device->getName(), device);
	}

	void AddPlayer::imGuiUpdate(const DeltaTime& deltaTime) {
		ImGui::PushFont(mwetris::Configuration::getInstance().getImGuiHeaderFont());
		ImGui::Text("Add Player");
		ImGui::PopFont();
		
		ImGui::SetNextItemWidth(150.f);
		playerTypesCombo_.imGuiCombo("##Player Type");
		
		if (playerTypesCombo_.getSelected().player == Player::Human) {
			ImGui::SameLine();
			ImGui::SetNextItemWidth(150.f);
			if (humansCombo_.imGuiCombo("##Players")) {
				auto selected = humansCombo_.getSelected();
				selected.name = playerName_;
				player_ = selected;
			}
		} else if (playerTypesCombo_.getSelected().player == Player::Ai) {
			ImGui::SameLine();
			ImGui::SetNextItemWidth(150.f);
			if (aisCombo_.imGuiCombo("##Players")) {
				auto selected = aisCombo_.getSelected();
				selected.name = playerName_;
				player_ = selected;
			}
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
		playerName_ = fmt::format("Player {}", data_.index + 1);
		allDevices_ = getDeviceTypes(deviceManager_->getAllDevicesAvailable(), data_.usedDevices);

		playerTypesCombo_ = ImGui::ComboVector<PlayerType>{getPlayerTypes(allDevices_.empty())};
		humansCombo_ = ImGui::ComboVector<game::Human>{allDevices_};
		aisCombo_ = ImGui::ComboVector<game::Ai>{allAis_};

		if (allDevices_.size() > 0) {
			auto selected = humansCombo_.getSelected();
			selected.name = playerName_;
			player_ = selected;
		} else if (allAis_.size() > 0) {
			auto selected = aisCombo_.getSelected();
			selected.name = playerName_;
			player_ = selected;
		}
	}

	void AddPlayer::switchedFrom() {
	}

	const std::vector<AddPlayer::PlayerType>& AddPlayer::getPlayerTypes(bool onlyAi) {
		static const std::vector<PlayerType> playerTypes = {
			PlayerType{
				.player = Player::Human,
				.name = "Human"
			},
			PlayerType{
				.player = Player::Ai,
				.name = "AI"
			}
		};

		static const std::vector<PlayerType> onlyAiType = {
			PlayerType{
				.player = Player::Ai,
				.name = "AI"
			}
		};

		if (onlyAi) {
			return onlyAiType;
		}
		return playerTypes;
	}

	const std::vector<AddPlayer::GameMode>& AddPlayer::getGameModes() {
		static const std::vector<GameMode> gameModes = {
			GameMode{
				.name = "Standard Game",
			},
			GameMode{
				.name = "Something else",
			}
		};

		return gameModes;
	}

}
