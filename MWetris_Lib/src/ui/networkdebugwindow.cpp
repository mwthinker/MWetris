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

		debugServer_->addPlayerSlotsCallback([this](const std::vector<network::Slot>& playerSlots) {
			playerSlots_ = playerSlots;
		});

		debugServer_->addInitGameCallback([this](const game::InitGameEvent& initGameEvent) {
			gameComponent_->initGame(initGameEvent);
		});

		debugServer_->addClientListener([this](const network::ConnectedClient& client) {
			if (std::any_of(connectedClients_.begin(), connectedClients_.end(), [&client](const auto& connected) {
				return connected.clientId == client.clientId;
			})) {
				return;
			}

			connectedClients_.push_back(client);
		});
	}

	NetworkDebugWindow::~NetworkDebugWindow() = default;

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
		
		int id = 0;
		for (auto& connectedClient : connectedClients_) {
			ImGui::PushID(++id);
			ImGui::Text("Connected: %s", connectedClient.clientId.c_str());
			ImGui::PopID();
		}

		if (ImGui::Checkbox("Connected", &connect)) {
			static const std::string& uuid = "REMOTE_UUID";
			if (connect) {
				// TODO!
			} else {
				//debugServer_->disconnect(GameRoomId{uuid});
			}
		}
		bool pause = false;
		if (ImGui::Checkbox("Paused", &pause)) {
			// TODO!
		}
			
		if (ImGui::Button("Restart Game")) {
			// TODO!
		}

		ImGui::Separator();
		for (int i = 0; i < playerSlots_.size(); ++i) {
			auto& slot = playerSlots_[i];

			ImGui::PushID(i + 1);

			ImGui::BeginGroup();
			switch (slot.type) {
				case network::SlotType::Open:
					ImGui::Text("Open Slot");
					break;
				case network::SlotType::Remote:
					if (slot.ai) {
						ImGui::Text("Remote AI");
					} else {
						ImGui::Text("Remote Player");
					}
					ImGui::Text("Player name: %s", slot.name.c_str());
					ImGui::Text("Client UUID: %s", slot.clientId.c_str());
					ImGui::Text("Player UUID: %s", slot.playerId.c_str());
					break;
				case network::SlotType::Closed:
					ImGui::Text("Closed Slot");
					break;
			}
			ImGui::EndGroup();
			ImGui::Separator();
			ImGui::PopID();
		}
	}

}
