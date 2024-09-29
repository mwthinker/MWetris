#ifndef APP_UI_SCENE_CREATEGAMEROOM_H
#define APP_UI_SCENE_CREATEGAMEROOM_H

#include "scene.h"

#include "types.h"
#include "configuration.h"
#include "ui/imguiextra.h"

#include "game/serialize.h"
#include "game/device.h"

#include <tetris/ai.h>

#include <variant>
#include <vector>

namespace app {

	class TetrisController;

}

namespace app::ui::scene {

	class CreateGameRoom : public Scene {
	public:
		CreateGameRoom(std::shared_ptr<app::TetrisController> tetrisController);

		void imGuiUpdate(const DeltaTime& deltaTime) override;

	private:
		void switchedTo(const SceneData& sceneData) override;
		void switchedFrom() override;

		mw::signals::ScopedConnections connections_;
		std::shared_ptr<app::TetrisController> tetrisController_;
		std::string serverName_ = "";
		bool public_ = false;
	};

}

#endif
