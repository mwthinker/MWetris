#ifndef MWETRIS_UI_SCENE_ADDPLAYER_H
#define MWETRIS_UI_SCENE_ADDPLAYER_H

#include "scene.h"

#include "types.h"
#include "configuration.h"

#include "ui/imguiextra.h"

#include "game/serialize.h"
#include "game/device.h"
#include "game/remoteplayer.h"

#include <ai.h>

#include <variant>
#include <functional>

namespace mwetris::game {

	class TetrisGame;
	class DeviceManager;

}

namespace mwetris::ui::scene {

	struct DeviceType {
		std::string name;
		game::DevicePtr device;
	};

	struct AiType {
		std::string name;
		tetris::Ai ai;
	};

	struct InternetPlayer {
		game::RemotePlayerPtr remotePlayer;
	};

	struct OpenSlot {
	};

	using PlayerSlot = std::variant<DeviceType, AiType, InternetPlayer, OpenSlot>;

	struct AddPlayerData : public scene::SceneData {
		int index;
	};

	class AddPlayer : public Scene {
	public:

		AddPlayer(std::function<void(AddPlayer&)> addCallback, std::shared_ptr<game::DeviceManager> deviceManager);

		void imGuiUpdate(const DeltaTime& deltaTime) override;

		const PlayerSlot& getPlayer() const {
			return player_;
		}

		int getSlotIndex() const {
			return data_.index;
		}

	private:
		void deviceConnected(game::DevicePtr device);

		void switchedTo(const SceneData& sceneData) override;
		void switchedFrom() override;

		std::function<void(AddPlayer&)> addCallback_;
		std::shared_ptr<game::DeviceManager> deviceManager_;

		std::vector<DeviceType> allDevices_;
		std::vector<AiType> allAis_;

		std::string playerName_;
		tetris::Ai ais_;
		PlayerSlot player_;
		AddPlayerData data_{};

		mw::signals::ScopedConnections connections_;
	};

}

#endif
