#ifndef MWETRIS_UI_SCENE_CUSTOMGAME_H
#define MWETRIS_UI_SCENE_CUSTOMGAME_H

#include "scene.h"

#include "types.h"
#include "configuration.h"
#include "ui/imguiextra.h"
#include "game/serialize.h"

namespace mwetris::ui::scene {

	class CustomGame : public Scene {
	public:
		CustomGame(std::shared_ptr<game::TetrisGame> tetrisGame, std::shared_ptr<game::DeviceManager> deviceManager);

		void imGuiUpdate(const DeltaTime& deltaTime) override;

	private:
		void switchedFrom() override;

		std::shared_ptr<game::TetrisGame> tetrisGame_;
		std::shared_ptr<game::DeviceManager> deviceManager_;

		std::vector<game::DevicePtr> devices_;
	};

}

#endif
