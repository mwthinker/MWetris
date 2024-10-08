#include "gameroomlobby.h"
#include "addplayer.h"
#include "tetriscontroller.h"

#include "game/devicemanager.h"
#include "game/tetrisgame.h"
#include "cnetwork/network.h"
#include "ui/imguiextra.h"
#include "util/auxiliary.h"

#include <sdl/color.h>

#include <array>
#include <string>
#include <map>
#include <concepts> 
#include <variant>

namespace app::ui::scene {

	namespace {

		int acceptNameInput(ImGuiInputTextCallbackData* data) {
			return data->BufTextLen < 30;
		}

		constexpr auto PopUpId = "CreatePopup";

		std::vector<game::PlayerPtr> createPlayersFromSlots(const std::vector<game::PlayerSlot>& playerSlots) {
			std::vector<game::PlayerPtr> players;
			for (const auto& playerSlot : playerSlots) {
				if (auto human = std::get_if<game::Human>(&playerSlot)) {
					players.push_back(game::createHumanPlayer(human->device));
				} else if (auto ai = std::get_if<game::Ai>(&playerSlot)) {
					players.push_back(game::createAiPlayer(ai->ai));
				}
			}
			return players;
		}

		void imGuiGameRulesConfigUpdate(game::DefaultGameRules::Config& config) {
			ImGui::Text("DefaultGameRules");
			//ImGui::InputInt("Level Up Nbr", &gameRulesConfig.levelUpNbr);
		}

		void imGuiGameRulesConfigUpdate(game::SurvivalGameRules::Config& config) {
			ImGui::Text("SurvivalGameRules");
		}

		std::vector<game::DevicePtr> getUsedDevices(const std::vector<game::PlayerSlot>& playerSlots) {
			std::vector<game::DevicePtr> usedDevices;
			for (const auto& playerSlot : playerSlots) {
				if (auto human = std::get_if<game::Human>(&playerSlot)) {
					usedDevices.push_back(human->device);
				}
			}
			return usedDevices;
		}

		sdl::Color getColor(int connectionId) {
			switch (connectionId) {
				case 0: return sdl::color::html::Cyan;
				case 1: return sdl::color::html::Blue;
				case 2: return sdl::color::html::Orange;
				case 3: return sdl::color::html::Yellow;
				case 4: return sdl::color::html::Green;
				case 5: return sdl::color::html::Purple;
				case 6: return sdl::color::html::Red;
			}
			return sdl::color::Red;
		}

	}

	GameRoomLooby::GameRoomLooby(std::shared_ptr<TetrisController> tetrisController, std::shared_ptr<game::DeviceManager> deviceManager)
		: tetrisController_{tetrisController} {

		for (int i = 0; i < 4; ++i) {
			playerSlots_.push_back(game::OpenSlot{});
		}

		connections_ += tetrisController_->tetrisEvent.connect([this](const TetrisEvent& tetrisEvent) {
			if (auto playerSlotEvent = std::get_if<PlayerSlotEvent>(&tetrisEvent)) {
				onPlayerSlotEvent(*playerSlotEvent);
			}
		});

		addPlayer_ = std::make_unique<AddPlayer>([this](AddPlayer& addPlayer) {
			auto player = addPlayer.getPlayer();
			playerSlots_[addPlayer.getSlotIndex()] = player;
			tetrisController_->setPlayerSlot(player, addPlayer.getSlotIndex());
		}, deviceManager);
	}

	void GameRoomLooby::onPlayerSlotEvent(const PlayerSlotEvent& playerSlotEvent) {
		playerSlots_[playerSlotEvent.slot] = playerSlotEvent.playerSlot;
	}

	void GameRoomLooby::imGuiUpdate(const DeltaTime& deltaTime) {
		float width = ImGui::GetWindowWidth();

		float sideX = 10.f;

		float slotSize = 100.f;
		float deltaX = 10.f;
		if (playerSlots_.size() > 1) {
			width = (width - playerSlots_.size() * slotSize) / (playerSlots_.size() + 1);
		}

		auto pos = ImGui::GetCursorScreenPos();
		pos.y += 10.f;
		pos.x += deltaX;

		for (int i = 0; i < playerSlots_.size(); ++i) {
			auto& playerSlot = playerSlots_[i];

			ImGui::PushID(i + 1);
			pos.x += width;
			ImGui::SetCursorScreenPos(pos);
			pos.x += slotSize;

			ImGui::BeginGroup();
			std::visit([&](auto&& slot) mutable {
				using T = std::decay_t<decltype(slot)>;
				if constexpr (std::is_same_v<T, game::Human>) {
					ImGui::Text("game::Human");
					ImGui::Text("Player name: %s", slot.name.c_str());
					if (ImGui::AbortButton("Remove")) {
						playerSlot = game::OpenSlot{};
						tetrisController_->setPlayerSlot(playerSlot, i);
					}
				} else if constexpr (std::is_same_v<T, game::Ai>) {
					ImGui::Text("game::Ai");
					ImGui::Text("Player name: %s", slot.name.c_str());
					if (ImGui::AbortButton("Remove")) {
						playerSlot = game::OpenSlot{};
						tetrisController_->setPlayerSlot(playerSlot, i);
					}
				} else if constexpr (std::is_same_v<T, game::Remote>) {
					ImGui::Text("game::Remote");
					if (ImGui::AbortButton("Remove")) {
						playerSlot = game::OpenSlot{};
						tetrisController_->setPlayerSlot(playerSlot, i);
					}
				} else if constexpr (std::is_same_v<T, game::ClosedSlot>) {
					// Skip.
				} else if constexpr (std::is_same_v<T, game::OpenSlot>) {
					if (ImGui::Button("Open Slot", {slotSize, slotSize})) {
						scene::AddPlayerData data{};
						data.index = i;
						data.usedDevices = getUsedDevices(playerSlots_);
						addPlayer_->switchedTo(data);

						ImGui::OpenPopup(PopUpId);
						ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, {0.5f, 0.5f});
						auto size = addPlayer_->getSize();
						if (size.x > 0 && size.y > 0) {
							ImGui::SetNextWindowSize(size, ImGuiCond_Appearing);
						} else {
							ImGui::SetNextWindowSize({800, 800}, ImGuiCond_Appearing);
						}
					}
				} else {
					static_assert(util::always_false_v<T>, "non-exhaustive visitor!");
				}
			}, playerSlot);
			ImGui::EndGroup();

			ImGui::SetCursorPosY(150);

			ImGui::Table("Highscore", 2, ImGuiTableFlags_Borders, {500, 0}, [&]() {
				ImGui::TableSetupColumn("Connection", ImGuiTableColumnFlags_WidthFixed, 90);
				ImGui::TableSetupColumn("Id");

				ImGui::TableHeadersRow();
				for (const auto& client : tetrisController_->getGameRoomClients()) {
					ImGui::TableNextRow();
					ImGui::TableNextColumn(); ImGui::TextWithBackgroundColor(client.connectionId, getColor(client.connectionId));
					ImGui::TableNextColumn(); ImGui::Text("%s", client.clientId.c_str());
				}
			});

			static int gameRulesConfigIndex = 0;
			if (ImGui::RadioButton("Default Game rules", &gameRulesConfigIndex, 0)) {
				gameRulesConfig_ = game::DefaultGameRules::Config{};
			}
			if (ImGui::RadioButton("Survival Game rules", &gameRulesConfigIndex, 1)) {
				gameRulesConfig_ = game::SurvivalGameRules::Config{};
			}
			std::visit([&](auto&& config) mutable {
				imGuiGameRulesConfigUpdate(config);
			}, gameRulesConfig_);

			ImGui::PopupModal(PopUpId, nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar, [&]() {
				auto pos = ImGui::GetCursorScreenPos();
				auto size = ImVec2{60.f, 30.f};

				ImGui::SetCursorPosX(ImGui::GetWindowWidth() - size.x - 10.f); // TODO! Fix more less hard coded right alignment.
				if (ImGui::AbortButton("Cancel", size) || ImGui::IsKeyDown(ImGuiKey_Escape)) {
					ImGui::CloseCurrentPopup();
				}
				ImGui::SetCursorScreenPos(pos);

				addPlayer_->imGuiUpdate(deltaTime);
			});

			ImGui::PopID();
		}
		if (gameRoomSceneData_.type == GameRoomLoobyData::Type::Network) {
			gameRoomId_ = tetrisController_->getGameRoomId();
			if (!gameRoomId_.empty()) {
				ImGui::SetCursorPosY(250);
				ImGui::Separator();
				ImGui::Text("Server Id: ");
				ImGui::SameLine();
				ImGui::InputText("##ServerId", &gameRoomId_, ImGuiInputTextFlags_ReadOnly);
			}
		}

		width = ImGui::GetWindowWidth() - 2 * ImGui::GetCursorPosX();
		float height = 100.f;
		float y = ImGui::GetWindowHeight() - height - ImGui::GetStyle().WindowPadding.y;

		ImGui::BeginDisabled(playersInSlots(playerSlots_) == 0);
		ImGui::SetCursorPosY(y);

		if (ImGui::ConfirmationButton("Create Game", {width, height})) {
			if (!gameRoomId_.empty() && gameRoomSceneData_.type == GameRoomLoobyData::Type::Network) {
				spdlog::info("[CreateGame] Starting network game.");
				tetrisController_->startNetworkGame(gameRulesConfig_, game::TetrisWidth, game::TetrisHeight);
			} else {
				spdlog::info("[CreateGame] Starting local game.");
				tetrisController_->startLocalGame(gameRulesConfig_, createPlayersFromSlots(playerSlots_));
			}
		}
		ImGui::EndDisabled();
	}

	void GameRoomLooby::switchedTo(const SceneData& sceneData) {
		gameRulesConfig_ = game::DefaultGameRules::Config{};
		try {
			gameRoomSceneData_ = dynamic_cast<const GameRoomLoobyData&>(sceneData);
		} catch (const std::bad_cast& exp) {
			gameRoomSceneData_ = {};
			spdlog::error("Bug, should be type CreateGameData: {}", exp.what());
		}
		reset(playerSlots_);
	}

	void GameRoomLooby::switchedFrom() {
	}

}
