#ifndef MWETRIS_UI_SCENE_JOINGAME_H
#define MWETRIS_UI_SCENE_JOINGAME_H

#include "scene.h"

#include "types.h"
#include "configuration.h"

#include "ui/imguiextra.h"

#include "game/serialize.h"
#include "game/device.h"

#include <ai.h>

#include <variant>

namespace mwetris::game {

	class TetrisGame;
	class DeviceManager;

}

namespace mwetris::ui::scene {

	class JoinGame : public Scene {
	public:
		JoinGame(std::shared_ptr<game::TetrisGame> tetrisGame, std::shared_ptr<game::DeviceManager> deviceManager);

		void imGuiUpdate(const DeltaTime& deltaTime) override;

	private:
		void deviceConnected(game::DevicePtr device);

		void switchedTo(const SceneData& sceneData) override;
		void switchedFrom() override;

		std::shared_ptr<game::TetrisGame> tetrisGame_;
		std::shared_ptr<game::DeviceManager> deviceManager_;
		std::string serverId_ = "";
		mw::signals::ScopedConnections connections_;
	};

}

#endif
