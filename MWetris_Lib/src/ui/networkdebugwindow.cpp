#include "networkdebugwindow.h"
#include "util.h"

#include "network/debugclient.h"

namespace mwetris::ui {

	NetworkDebugWindow::NetworkDebugWindow(std::shared_ptr<network::DebugClient> client)
		: debugClient_{client} {

		debugClient_->addPlayerSlotsCallback([this](const std::vector<game::PlayerSlot>& playerSlots) {
			playerSlots_ = playerSlots;
		});
	}

	void NetworkDebugWindow::setVisible(bool visible) {
		visible_ = visible;
	}

	bool NetworkDebugWindow::isVisible() const {
		return visible_;
	}
		
	void NetworkDebugWindow::imGuiUpdate(const sdl::DeltaTime& deltaTime) {
		if (visible_) {
			ImGui::SetNextWindowSize({650, 650});
			ImGui::PushStyleColor(ImGuiCol_WindowBg, {0, 0, 0, 1});
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {10, 10});
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 10.f);

			ImGui::Window("Debug Network Window", [&]() {
				update();
			});

			ImGui::PopStyleVar(2);
			ImGui::PopStyleColor();
		}
	}

	void NetworkDebugWindow::update() {
		static bool connect = false;
		if (ImGui::Checkbox("Connected", &connect)) {
			static const std::string& uuid = "REMOTE_UUID";
			if (connect) {
				debugClient_->connect(uuid);
			} else {
				debugClient_->disconnect(uuid);
			}
		}
		bool pause = debugClient_->isPaused();
		if (ImGui::Checkbox("Paused", &pause)) {
			debugClient_->sendPause(!pause);
		}
			
		if (ImGui::Button("Restart Game")) {
			debugClient_->restartGame();
		}

		for (int i = 0; i < playerSlots_.size(); ++i) {
			auto& playerSlot = playerSlots_[i];

			ImGui::PushID(i + 1);

			ImGui::BeginGroup();
			std::visit([&](auto&& slot) mutable {
				using T = std::decay_t<decltype(slot)>;
				if constexpr (std::is_same_v<T, game::Human>) {
					ImGui::Text("game::Human");
					ImGui::Text("Player name: %s", slot.name.c_str());
				} else if constexpr (std::is_same_v<T, game::Ai>) {
					ImGui::Text("game::Ai");
					ImGui::Text("Player name: %s", slot.name.c_str());
				} else if constexpr (std::is_same_v<T, game::Remote>) {
					ImGui::Text("game::Remote");
					ImGui::Text("Player name: %s", slot.name.c_str());
					ImGui::Text("Is AI: %s", slot.ai ? "true" : "false");
				} else if constexpr (std::is_same_v<T, game::ClosedSlot>) {
					// Skip.
				} else if constexpr (std::is_same_v<T, game::OpenSlot>) {
					if (ImGui::Button("Open Slot", {100, 100})) {
							
					}
				} else {
					static_assert(always_false_v<T>, "non-exhaustive visitor!");
				}
			}, playerSlot);
			ImGui::EndGroup();

			ImGui::PopID();
		}
		/*

		if (ImGui::Button("Remote player Slot 0")) {
				
			//network_.setPlayerSlot(game::Remote{}, 0);
		}
		ImGui::SameLine();
		if (ImGui::Button("Remote player Slot 1")) {
			//network_.setPlayerSlot(game::Remote{}, 1);
		}
		ImGui::SameLine();
		if (ImGui::Button("Remote player Slot 2")) {
			//network_.setPlayerSlot(game::Remote{}, 2);
		}
		ImGui::SameLine();
		if (ImGui::Button("Remote player Slot 3")) {
			//network_.setPlayerSlot(game::Remote{}, 3);
		}
		ImGui::SameLine();
		*/
	}

}
