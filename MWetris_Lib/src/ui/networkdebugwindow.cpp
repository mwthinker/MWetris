#include "networkdebugwindow.h"
#include "util.h"

#include "network/debugclient.h"

#include "graphic/gamecomponent.h"

namespace mwetris::ui {

	namespace {

		constexpr double toSeconds(const auto& duration) {
			return std::chrono::duration<double>(duration).count();
		}

	}

	NetworkDebugWindow::NetworkDebugWindow(std::shared_ptr<network::DebugServer> server)
		: debugServer_{server}
		, gameComponent_{std::make_unique<graphic::GameComponent>()} {

		debugServer_->addPlayerSlotsCallback([this](const std::vector<game::PlayerSlot>& playerSlots) {
			playerSlots_ = playerSlots;
		});

		debugServer_->addInitGameCallback([this](const game::InitGameEvent& initGameEvent) {
			gameComponent_->initGame(initGameEvent);
		});
	}
		
	void NetworkDebugWindow::imGuiUpdate(const sdl::DeltaTime& deltaTime) {
		ImGui::SetNextWindowSize({650, 650});
		ImGui::PushStyleColor(ImGuiCol_WindowBg, sdl::color::html::Black);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {10, 10});
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 10.f);

		ImGui::Window("Debug Network Window", [&]() {
			update();
		});

		ImGui::SetNextWindowSize({400, 600});
		ImGui::Window("Server", [&]() {
			auto size = ImGui::GetContentRegionAvail();
			gameComponent_->draw(size.x, size.y, toSeconds(deltaTime));
		});

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor();
	}

	void NetworkDebugWindow::update() {
		static bool connect = false;
		if (ImGui::Checkbox("Connected", &connect)) {
			static const std::string& uuid = "REMOTE_UUID";
			if (connect) {
				debugServer_->connect(uuid);
			} else {
				debugServer_->disconnect(uuid);
			}
		}
		bool pause = debugServer_->isPaused();
		if (ImGui::Checkbox("Paused", &pause)) {
			debugServer_->sendPause(!pause);
		}
			
		if (ImGui::Button("Restart Game")) {
			debugServer_->restartGame();
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
	}

}
