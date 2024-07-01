#ifndef MWETRIS_UI_SCENE_ADDPLAYER_H
#define MWETRIS_UI_SCENE_ADDPLAYER_H

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

namespace mwetris::game {

	class TetrisGame;
	class DeviceManager;

}

namespace mwetris::ui::scene {

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
	
	private:
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

		mw::signals::ScopedConnections connections_;
	};

}

#endif
