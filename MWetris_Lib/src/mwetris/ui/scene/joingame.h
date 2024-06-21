#ifndef MWETRIS_UI_SCENE_JOINGAME_H
#define MWETRIS_UI_SCENE_JOINGAME_H

#include "scene.h"

#include "types.h"
#include "configuration.h"
#include "ui/imguiextra.h"

#include "game/serialize.h"
#include "game/device.h"
#include "tetriscontroller.h"

#include <tetris/ai.h>

#include <variant>
#include <vector>

namespace mwetris {

	namespace game {

		class DeviceManager;
		class TetrisGame;
		class Network;

	}

	namespace network {

		class Network;

	}

}

namespace mwetris::ui::scene {

	class AddPlayer;

	class JoinGame : public Scene {
	public:
		JoinGame(std::shared_ptr<TetrisController> tetrisController);

		void imGuiUpdate(const DeltaTime& deltaTime) override;

	private:
		void switchedTo(const SceneData& sceneData) override;
		void switchedFrom() override;

		std::shared_ptr<TetrisController> tetrisController_;
		std::string serverId_ = "";
		std::string filter_ = "";
		mw::signals::ScopedConnections connections_;
		std::vector<network::GameRoomListEvent::GameRoom> gameRooms_;
	};

}

#endif
