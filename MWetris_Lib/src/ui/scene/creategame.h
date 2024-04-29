#ifndef MWETRIS_UI_SCENE_CREATEGAME_H
#define MWETRIS_UI_SCENE_CREATEGAME_H

#include "scene.h"

#include "types.h"
#include "configuration.h"

#include "ui/imguiextra.h"

#include "game/serialize.h"
#include "game/device.h"
#include "game/playerslot.h"

#include <ai.h>

#include <variant>
#include <memory>

namespace mwetris::game {

	class TetrisGame;
	class Network;

}

namespace mwetris::network {

	class Network;

}

namespace mwetris::ui::scene {

	class AddPlayer;

	class CreateGame : public Scene {
	public:
		CreateGame(
			std::shared_ptr<game::TetrisGame> game,
			std::shared_ptr<network::Network> network,
			std::shared_ptr<game::DeviceManager> deviceManager
		);

		void imGuiUpdate(const DeltaTime& deltaTime) override;

	private:
		void switchedTo(const SceneData& sceneData) override;
		void switchedFrom() override;

		std::shared_ptr<network::Network> network_;
		std::shared_ptr<game::TetrisGame> game_;
		std::string serverId_ = "";
		mw::signals::ScopedConnections connections_;

		std::vector<game::PlayerSlot> playerSlots_;
		bool openPopUp_ = false;
		std::unique_ptr<AddPlayer> addPlayer_;
		std::shared_ptr<game::DeviceManager> deviceManager_;
		std::string gameRoomUuid_;
	};

}

#endif
