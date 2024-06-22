#ifndef MWETRIS_UI_SCENE_CREATEGAMEROOM_H
#define MWETRIS_UI_SCENE_CREATEGAMEROOM_H

#include "scene.h"

#include "types.h"
#include "configuration.h"
#include "ui/imguiextra.h"

#include "game/serialize.h"
#include "game/device.h"

#include <tetris/ai.h>

#include <variant>
#include <vector>

namespace mwetris {

	class TetrisController;

	namespace network {

		class Network;

	}

}

namespace mwetris::ui::scene {

	class CreateGameRoom : public Scene {
	public:
		CreateGameRoom(std::shared_ptr<mwetris::TetrisController> tetrisController);

		void imGuiUpdate(const DeltaTime& deltaTime) override;

	private:
		void switchedTo(const SceneData& sceneData) override;
		void switchedFrom() override;

		mw::signals::ScopedConnections connections_;
		std::shared_ptr<mwetris::TetrisController> tetrisController_;
		std::string serverName_ = "";
		bool public_ = false;
	};

}

#endif
