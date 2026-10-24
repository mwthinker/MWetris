#include "gameroomlobby.h"
#include "addplayer.h"
#include "tetriscontroller.h"

#include "game/devicemanager.h"
#include "game/tetrisgame.h"
#include "cnetwork/network.h"
#include "ui/imguiextra.h"
#include "ui/theme.h"
#include "util/auxiliary.h"

#include <sdl/color.h>

#include <algorithm>
#include <array>
#include <string>
#include <map>
#include <concepts> 
#include <variant>

namespace app::ui::scene {

	namespace {

		constexpr auto PopUpId = "CreatePopup";

		std::vector<game::PlayerPtr> createPlayersFromSlots(const std::vector<game::PlayerSlot>& playerSlots, const std::shared_ptr<ControlProfiles>& controlProfiles) {
			std::vector<game::PlayerPtr> players;
			for (const auto& playerSlot : playerSlots) {
				if (auto human = std::get_if<game::Human>(&playerSlot)) {
					players.push_back(game::createHumanPlayer(human->device, controlProfiles));
				} else if (auto ai = std::get_if<game::Ai>(&playerSlot)) {
					players.push_back(game::createAiPlayer(ai->ai));
				}
			}
			return players;
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

	GameRoomLooby::GameRoomLooby(std::shared_ptr<TetrisController> tetrisController, std::shared_ptr<game::DeviceManager> deviceManager, std::shared_ptr<ControlProfiles> controlProfiles)
		: tetrisController_{tetrisController}
		, controlProfiles_{std::move(controlProfiles)} {

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
		const bool networkLobby = gameRoomSceneData_.type == GameRoomLoobyData::Type::Network;
		const float backButtonWidth = 90.f;

		ImGui::PushFont(Configuration::getInstance().getImGuiSemiBoldFont(), 0.f);
		ImGui::TextUnformatted(networkLobby ? "Online Lobby" : "Local Multiplayer");
		ImGui::PopFont();
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() - backButtonWidth - ImGui::GetStyle().WindowPadding.x);
		if (ImGui::MediumButton("Back", {backButtonWidth, 30.f})) {
			tetrisController_->leaveGameRoom();
		}
		ImGui::Spacing();
		ImGui::SecondaryText(networkLobby ? "Invite players and configure the match." : "Add local players and configure the match.");
		ImGui::Dummy({0.f, 4.f});
		ImGui::Separator();
		ImGui::Dummy({0.f, 6.f});

		auto drawPlayerCards = [&]() {
			ImGui::PushFont(Configuration::getInstance().getImGuiMediumFont(), 0.f);
			ImGui::TextUnformatted("Player Layout");
			ImGui::PopFont();
			ImGui::SecondaryText("Players appear in this left-to-right order during the game.");
			ImGui::Dummy({0.f, 6.f});

			const int columns = static_cast<int>(playerSlots_.size());
			const bool compactCards = ImGui::GetContentRegionAvail().x / columns < 150.f;
			if (ImGui::BeginTable("Player cards", columns, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_PadOuterX)) {
				for (int i = 0; i < playerSlots_.size(); ++i) {
					auto& playerSlot = playerSlots_[i];
					ImGui::TableNextColumn();
					ImGui::PushID(i);
					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {12.f, 10.f});
					if (ImGui::BeginChild("Player card", {0.f, 125.f}, true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
						ImGui::Text(compactCards ? "P%d" : "Player %d", i + 1);
						ImGui::Separator();
						ImGui::Spacing();
						std::visit([&](auto&& slot) {
							using T = std::decay_t<decltype(slot)>;
							if constexpr (std::is_same_v<T, game::Human>) {
								ImGui::TextUnformatted(slot.name.c_str());
								const char* deviceName = slot.device ? slot.device->getName().c_str() : "No device";
								ImGui::SecondaryText("Human - %s", deviceName);
								if (ImGui::MediumButton(compactCards ? "Remove##Human" : "Remove", {-1.f, 28.f})) {
									playerSlot = game::OpenSlot{};
									tetrisController_->setPlayerSlot(playerSlot, i);
								}
							} else if constexpr (std::is_same_v<T, game::Ai>) {
								ImGui::TextUnformatted(slot.name.c_str());
								ImGui::SecondaryText("Computer player");
								if (ImGui::MediumButton(compactCards ? "Remove##Ai" : "Remove", {-1.f, 28.f})) {
									playerSlot = game::OpenSlot{};
									tetrisController_->setPlayerSlot(playerSlot, i);
								}
							} else if constexpr (std::is_same_v<T, game::Remote>) {
								ImGui::TextUnformatted(slot.name.c_str());
								ImGui::SecondaryText(slot.ai ? "Remote computer" : "Remote player");
								if (ImGui::MediumButton(compactCards ? "Remove##Remote" : "Remove", {-1.f, 28.f})) {
									playerSlot = game::OpenSlot{};
									tetrisController_->setPlayerSlot(playerSlot, i);
								}
							} else if constexpr (std::is_same_v<T, game::ClosedSlot>) {
								ImGui::BeginDisabled();
								ImGui::MediumButton("Closed", {-1.f, 65.f});
								ImGui::EndDisabled();
							} else if constexpr (std::is_same_v<T, game::OpenSlot>) {
								if (ImGui::MediumButton(compactCards ? "+ Add" : "+ Add Player", {-1.f, 65.f})) {
									AddPlayerData data{};
									data.index = i;
									data.usedDevices = getUsedDevices(playerSlots_);
									addPlayer_->switchedTo(data);
									openPopUp_ = true;
								}
							} else {
								static_assert(util::always_false_v<T>, "non-exhaustive visitor!");
							}
						}, playerSlot);
					}
					ImGui::EndChild();
					ImGui::PopStyleVar();
					ImGui::PopID();
				}
				ImGui::EndTable();
			}
		};

		auto drawGameSettings = [&]() {
			ImGui::PushFont(Configuration::getInstance().getImGuiMediumFont(), 0.f);
			ImGui::TextUnformatted("Game Settings");
			ImGui::PopFont();
			int rulesIndex = std::holds_alternative<game::DefaultGameRules::Config>(gameRulesConfig_) ? 0 : 1;
			ImGui::PushStyleColor(ImGuiCol_CheckMark, theme::Success.toImU32());
			if (ImGui::RadioButton("Classic", &rulesIndex, 0)) {
				gameRulesConfig_ = game::DefaultGameRules::Config{};
			}
			ImGui::SecondaryText("Clear lines and increase your score.");
			if (ImGui::RadioButton("Survival", &rulesIndex, 1)) {
				gameRulesConfig_ = game::SurvivalGameRules::Config{};
			}
			ImGui::PopStyleColor();
			ImGui::SecondaryText("Outlast the other players.");

			if (networkLobby) {
				gameRoomId_ = tetrisController_->getGameRoomId();
				ImGui::Spacing();
				ImGui::SeparatorText("Room");
				if (!gameRoomId_.empty()) {
					ImGui::SetNextItemWidth(std::max(80.f, ImGui::GetContentRegionAvail().x - 70.f));
					ImGui::InputText("##RoomId", &gameRoomId_, ImGuiInputTextFlags_ReadOnly);
					ImGui::SameLine();
					if (ImGui::MediumButton("Copy", {60.f, 0.f})) {
						ImGui::SetClipboardText(gameRoomId_.c_str());
					}
				}

				const auto& clients = tetrisController_->getGameRoomClients();
				ImGui::Text("Connected: %d", static_cast<int>(clients.size()));
				for (const auto& client : clients) {
					ImGui::TextWithBackgroundColor(client.connectionId, getColor(client.connectionId), {3.f, 1.f});
					ImGui::SameLine();
					ImGui::TextUnformatted(client.clientId.c_str());
				}
			}
		};

		if (ImGui::BeginChild("Lobby content", {0.f, -65.f})) {
			drawPlayerCards();
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			drawGameSettings();
		}
		ImGui::EndChild();

		ImGui::Separator();
		const int playerCount = playersInSlots(playerSlots_);
		const bool waitingForRoom = networkLobby && gameRoomId_.empty();
		const bool canStart = playerCount > 0 && !waitingForRoom;
		if (waitingForRoom) {
			ImGui::SecondaryText("Waiting for the online room...");
		} else if (playerCount == 0) {
			ImGui::SecondaryText("Add at least one player to start.");
		} else {
			ImGui::Text("%d player%s ready", playerCount, playerCount == 1 ? "" : "s");
		}
		ImGui::SameLine();
		ImGui::AlignNextItemRight(140.f);
		ImGui::BeginDisabled(!canStart);
		if (ImGui::ConfirmationButton("Start Game", {140.f, 40.f})) {
			if (networkLobby) {
				spdlog::info("[CreateGame] Starting network game.");
				tetrisController_->startNetworkGame(gameRulesConfig_, game::TetrisWidth, game::TetrisHeight);
			} else {
				spdlog::info("[CreateGame] Starting local game.");
				tetrisController_->startLocalGame(gameRulesConfig_, createPlayersFromSlots(playerSlots_, controlProfiles_));
			}
		}
		ImGui::EndDisabled();

		if (openPopUp_) {
			openPopUp_ = false;
			ImGui::OpenPopup(PopUpId);
		}
		ImGui::SetNextModalWindow(addPlayer_->getSize());
		ImGui::PushContentStyle();
		ImGui::PopupModal(PopUpId, nullptr,
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar
				| ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse, [&]() {
			auto pos = ImGui::GetCursorScreenPos();
			const auto closeButtonLength = ImGui::GetFrameHeight();
			const ImVec2 closeButtonSize{closeButtonLength, closeButtonLength};
			ImGui::SetCursorPosX(ImGui::GetWindowWidth() - closeButtonSize.x - ImGui::GetStyle().WindowPadding.x);
			if (ImGui::CloseIconButton(closeButtonSize) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::SetCursorScreenPos(pos);
			addPlayer_->imGuiUpdate(deltaTime);
		});
		ImGui::PopContentStyle();
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
