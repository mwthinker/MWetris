#include "creategame.h"
#include "../imguiextra.h"

#include "game/tetrisgame.h"
#include "network/network.h"
#include "ui/scene/addplayer.h"
#include "util.h"
#include "addplayer.h"

#include <array>
#include <string>
#include <map>
#include <concepts> 
#include <variant>

namespace mwetris::ui::scene {

	namespace {

		int acceptNameInput(ImGuiInputTextCallbackData* data) {
			return data->BufTextLen < 30;
		}

		constexpr auto popUpId = "Popup";

	}

	CreateGame::CreateGame(std::shared_ptr<game::TetrisGame> game, std::shared_ptr<network::Network> network, std::shared_ptr<game::DeviceManager> deviceManager)
		: network_{network}
		, game_{game}
		, deviceManager_{deviceManager} {

		for (int i = 0; i < 4; ++i) {
			playerSlots_.push_back(game::OpenSlot{});
		}

		connections_ += network_->addPlayerSlotListener([this](game::PlayerSlot slot, int index) {
			playerSlots_[index] = slot;
		});

		addPlayer_ = std::make_unique<AddPlayer>([this](AddPlayer& addPlayer) {
			auto player = addPlayer.getPlayer();
			playerSlots_[addPlayer.getSlotIndex()] = player;
			network_->setPlayerSlot(player, addPlayer.getSlotIndex());
		}, deviceManager_);
	}

	void CreateGame::imGuiUpdate(const DeltaTime& deltaTime) {
		float width = ImGui::GetWindowWidth();

		if (playerSlots_.size() > 1) {
			width = width / playerSlots_.size();
		}

		auto pos = ImGui::GetCursorScreenPos();

		for (int i = 0; i < playerSlots_.size(); ++i) {
			auto& playerSlot = playerSlots_[i];

			ImGui::PushID(i + 1);
			ImGui::SetCursorScreenPos(pos);
			pos.x += width;

			ImGui::BeginGroup();
			std::visit([&](auto&& slot) mutable {
				using T = std::decay_t<decltype(slot)>;
				if constexpr (std::is_same_v<T, game::Human>) {
					ImGui::Text("game::Human");
					ImGui::Text("Player name: %s", slot.name.c_str());
					if (ImGui::AbortButton("Remove")) {
						playerSlot = game::OpenSlot{};
						network_->setPlayerSlot(playerSlot, i);
					}
				} else if constexpr (std::is_same_v<T, game::Ai>) {
					ImGui::Text("game::Ai");
					ImGui::Text("Player name: %s", slot.name.c_str());
					if (ImGui::AbortButton("Remove")) {
						playerSlot = game::OpenSlot{};
						network_->setPlayerSlot(playerSlot, i);
					}
				} else if constexpr (std::is_same_v<T, game::Remote>) {
					ImGui::Text("game::Remote");
					if (ImGui::AbortButton("Remove")) {
						playerSlot = game::OpenSlot{};
						network_->setPlayerSlot(playerSlot, i);
					}
				} else if constexpr (std::is_same_v<T, game::ClosedSlot>) {
					// Skip.
				} else if constexpr (std::is_same_v<T, game::OpenSlot>) {
					if (ImGui::Button("Open Slot", {100, 100})) {
						scene::AddPlayerData data{};
						data.index = i;
						addPlayer_->switchedTo(data);
						
						ImGui::OpenPopup(popUpId);
						ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, {0.5f, 0.5f});
						ImGui::SetNextWindowSize({800, 800}, ImGuiCond_Appearing);
					}
				} else {
					static_assert(always_false_v<T>, "non-exhaustive visitor!");
				}
			}, playerSlot);
			ImGui::EndGroup();

			
			if (!ImGui::PopupModal(popUpId, nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar, [&]() {
				auto pos = ImGui::GetCursorScreenPos();
				auto size = ImVec2{60.f, 30.f};

				ImGui::SetCursorPosX(ImGui::GetWindowWidth() - size.x - 10.f); // TODO! Fix more less hard coded right alignment.
				if (ImGui::AbortButton("Cancel", size) || ImGui::IsKeyDown(ImGuiKey_Escape)) {
					ImGui::CloseCurrentPopup();
				}
				ImGui::SetCursorScreenPos(pos);

				addPlayer_->imGuiUpdate(deltaTime);
			})) {
				
			}

			ImGui::PopID();
		}
		gameRoomUuid_ = network_->getGameRoomUuid();
		if (!gameRoomUuid_.empty()) {
			ImGui::SetCursorPosY(200);
			ImGui::Separator();
			ImGui::Text("Server Id: ");
			ImGui::SameLine();
			ImGui::InputText("##ServerId", &gameRoomUuid_, ImGuiInputTextFlags_ReadOnly);
		}

		width = ImGui::GetWindowWidth() - 2 * ImGui::GetCursorPosX();
		float height = 100.f;
		float y = ImGui::GetWindowHeight() - height - ImGui::GetStyle().WindowPadding.y;

		ImGui::BeginDisabled(playersInSlots(playerSlots_) == 0);
		ImGui::SetCursorPosY(y);

		if (ImGui::ConfirmationButton("Create Game", {width, height})) {
			if (!gameRoomUuid_.empty()) {
				network_->startGame(std::make_unique<game::SurvivalGameRules>(), TetrisWidth, TetrisHeight);
			} else {
				game_->createGame(
					std::make_unique<game::SurvivalGameRules>(),
					game::PlayerFactory{}
					.createPlayers(TetrisWidth, TetrisHeight
						, extract<game::Human>(playerSlots_)
						, extract<game::Ai>(playerSlots_)));
			}
		}
		ImGui::EndDisabled();
	}

	void CreateGame::switchedTo(const SceneData& sceneData) {
		reset(playerSlots_);
	}

	void CreateGame::switchedFrom() {
	}

}
