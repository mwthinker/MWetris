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
	class Network;

}

namespace mwetris::network {

	class Network;

}

namespace mwetris::ui::scene {

	class JoinGame : public Scene {
	public:
		JoinGame(std::shared_ptr<game::TetrisGame> game, std::shared_ptr<network::Network> network);

		void imGuiUpdate(const DeltaTime& deltaTime) override;

	private:
		void switchedTo(const SceneData& sceneData) override;
		void switchedFrom() override;

		std::shared_ptr<network::Network> network_;
		std::shared_ptr<game::TetrisGame> game_;
		std::string serverId_ = "";
		mw::signals::ScopedConnections connections_;
		bool connected_ = true;
	};

}

#endif
