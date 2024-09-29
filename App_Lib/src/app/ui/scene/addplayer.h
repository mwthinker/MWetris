#ifndef APP_UI_SCENE_ADDPLAYER_H
#define APP_UI_SCENE_ADDPLAYER_H

#include "scene.h"

#include "../../types.h"
#include "../../configuration.h"

#include "../imguiextra.h"

#include "../../game/serialize.h"
#include "../../game/device.h"
#include "../../game/playerslot.h"

#include <tetris/ai.h>

#include <variant>
#include <functional>

namespace app::game {

	class TetrisGame;
	class DeviceManager;

}

namespace app::ui::scene {

	struct AddPlayerData : public scene::SceneData {
		int index;
		std::vector<game::DevicePtr> usedDevices;
	};

	class AddPlayer : public Scene {
	public:
		AddPlayer(std::function<void(AddPlayer&)> addCallback, std::shared_ptr<game::DeviceManager> deviceManager);

		void imGuiUpdate(const DeltaTime& deltaTime) override;

		const game::PlayerSlot& getPlayer() const {
			return player_;
		}

		int getSlotIndex() const {
			return data_.index;
		}

		void switchedTo(const SceneData& sceneData) override;

		ImVec2 getSize() const override {
			return {800, 200};
		}
	
	private:
		enum class Player {
			Human,
			Ai
		};

		struct PlayerType {
			Player player;
			std::string name;
		};

		struct GameMode {
			std::string name;
		};

		static const std::vector<GameMode>& getGameModes();

		static const std::vector<PlayerType>& getPlayerTypes(bool onlyAi);

		void deviceConnected(game::DevicePtr device);

		void switchedFrom() override;

		std::function<void(AddPlayer&)> addCallback_;
		std::shared_ptr<game::DeviceManager> deviceManager_;

		std::vector<game::Human> allDevices_;
		std::vector<game::Ai> allAis_;

		std::string playerName_;
		tetris::Ai ais_;
		game::PlayerSlot player_;
		AddPlayerData data_{};
		ImGui::ComboVector<GameMode> gameModesCombo_;
		ImGui::ComboVector<PlayerType> playerTypesCombo_;
		ImGui::ComboVector<game::Human> humansCombo_;
		ImGui::ComboVector<game::Ai> aisCombo_;

		mw::signals::ScopedConnections connections_;
	};

}

#endif
