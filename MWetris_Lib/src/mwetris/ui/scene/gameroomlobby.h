#ifndef MWETRIS_UI_SCENE_GAMEROOMLOOBY_H
#define MWETRIS_UI_SCENE_GAMEROOMLOOBY_H

#include "scene.h"

#include "types.h"
#include "configuration.h"

#include "ui/imguiextra.h"

#include "game/serialize.h"
#include "game/playerslot.h"
#include "game/devicemanager.h"
#include "game/defaultgamerules.h"
#include "network/networkevent.h"

#include <tetris/ai.h>

#include <variant>
#include <memory>

namespace mwetris {

	class TetrisController;
	struct PlayerSlotEvent;

	namespace game {

		class Network;
		class DeviceManager;

	}

	namespace network {

		class Network;

	}

}

namespace mwetris::ui::scene {

	class AddPlayer;

	struct GameRoomLoobyData : public scene::SceneData {
		enum class Type {
			Local,
			Network
		};

		Type type = Type::Local;
	};

	class GameRoomLooby : public Scene {
	public:
		GameRoomLooby(std::shared_ptr<TetrisController> tetrisController, std::shared_ptr<game::DeviceManager> deviceManager);

		void imGuiUpdate(const DeltaTime& deltaTime) override;

	private:
		void onPlayerSlotEvent(const PlayerSlotEvent& playerSlotEvent);

		void switchedTo(const SceneData& sceneData) override;
		void switchedFrom() override;

		std::shared_ptr<TetrisController> tetrisController_;
		std::string serverId_ = "";
		mw::signals::ScopedConnections connections_;

		std::vector<game::PlayerSlot> playerSlots_;
		bool openPopUp_ = false;
		std::unique_ptr<AddPlayer> addPlayer_;
		std::string gameRoomId_;
		GameRoomLoobyData gameRoomSceneData_;
		game::GameRulesConfig gameRulesConfig_;
	};

}

#endif
