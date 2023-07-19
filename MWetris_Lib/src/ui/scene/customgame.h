#ifndef MWETRIS_UI_SCENE_CUSTOMGAME_H
#define MWETRIS_UI_SCENE_CUSTOMGAME_H

#include "scene.h"

#include "types.h"
#include "configuration.h"

#include "ui/imguiextra.h"

#include "game/serialize.h"
#include "game/device.h"

namespace mwetris::game {

	class TetrisGame;
	class DeviceManager;

}

namespace ImGui {

	class PlayerCombo {



	};

}

namespace mwetris::ui::scene {

	struct DeviceType {
		std::string name;
		game::DevicePtr device;
	};

	class CustomGame : public Scene {
	public:
		CustomGame(std::shared_ptr<game::TetrisGame> tetrisGame, std::shared_ptr<game::DeviceManager> deviceManager);

		void imGuiUpdate(const DeltaTime& deltaTime) override;

	private:
		void deviceConnected(game::DevicePtr device);

		void switchedTo(const SceneData& sceneData) override;
		void switchedFrom() override;

		std::shared_ptr<game::TetrisGame> tetrisGame_;
		std::shared_ptr<game::DeviceManager> deviceManager_;

		std::vector<DeviceType> remainingDevices_;
		std::vector<DeviceType> playerDevices_;
		std::vector<DeviceType> allDevices_;

		mw::signals::ScopedConnections connections_;
	};

}

#endif
