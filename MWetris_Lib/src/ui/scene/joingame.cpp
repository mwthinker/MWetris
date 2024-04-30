#include "joingame.h"
#include "addplayer.h"
#include "tetriscontroller.h"
#include "util.h"

#include "game/devicemanager.h"
#include "game/playerslot.h"
#include "game/tetrisgame.h"
#include "network/network.h"
#include "ui/imguiextra.h"

#include <array>
#include <string>
#include <map>
#include <concepts>
#include <variant>
#include <memory>

namespace mwetris::ui::scene {

	namespace {

		int acceptNameInput(ImGuiInputTextCallbackData* data) {
			return data->BufTextLen < 30;
		}

		constexpr auto popUpId = "Popup";

	}

	JoinGame::JoinGame(std::shared_ptr<TetrisController> tetrisController, std::shared_ptr<game::DeviceManager> deviceManager)
		: tetrisController_{tetrisController} {

		for (int i = 0; i < 4; ++i) {
			playerSlots_.push_back(game::OpenSlot{});
		}

		connections_ += tetrisController_->tetrisEvent.connect([this](const TetrisEvent& tetrisEvent) {
			if (auto joinGameRoomEvent = std::get_if<network::JoinGameRoomEvent>(&tetrisEvent)) {
				if (joinGameRoomEvent->join) {
					gameRoomJoined_ = true;
				} else {
					// TODO: If the server id is invalid, show a message.
				}
			} else if (auto playerSlotEvent = std::get_if<PlayerSlotEvent>(&tetrisEvent)) {
				onPlayerSlotEvent(*playerSlotEvent);
			}
		});

		addPlayer_ = std::make_unique<AddPlayer>([this](AddPlayer& addPlayer) {
			auto player = addPlayer.getPlayer();
			playerSlots_[addPlayer.getSlotIndex()] = player;
			tetrisController_->setPlayerSlot(player, addPlayer.getSlotIndex());
		}, deviceManager);
	}

	void JoinGame::imGuiUpdate(const DeltaTime& deltaTime) {
		ImGui::PushFont(mwetris::Configuration::getInstance().getImGuiHeaderFont());
		ImGui::Text("Custom Game");
		ImGui::PopFont();

		if (gameRoomJoined_) {
		
		} else {
			ImGui::Text("Server Id");
			if (ImGui::InputText("##ServerId", &serverId_, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CallbackAlways, acceptNameInput)) {

			}

			ImGui::PopupContextItem([&]() {
				if (ImGui::MenuItem("Paste", "CTRL+V")) {
					serverId_.assign(ImGui::GetClipboardText());
					ImGui::CloseCurrentPopup();
				}
			});

			if (ImGui::ConfirmationButton("Connect")) { // TODO! Handle lag and show a spinner.
				tetrisController_->joinGameRoom(serverId_);
			}
		}
	}

	void JoinGame::imGuiJoinedGameRoom(const DeltaTime& deltaTime) {
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
	}

	void JoinGame::switchedTo(const SceneData& sceneData) {
		reset(playerSlots_);
		gameRoomJoined_ = false;
	}

	void JoinGame::switchedFrom() {
	}

	void JoinGame::onPlayerSlotEvent(const PlayerSlotEvent& playerSlotEvent) {
		playerSlots_[playerSlotEvent.slot] = playerSlotEvent.playerSlot;
	}

}
