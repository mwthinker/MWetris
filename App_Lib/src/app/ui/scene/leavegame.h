#ifndef APP_UI_SCENE_LEAVEGAME_H
#define APP_UI_SCENE_LEAVEGAME_H

#include "scene.h"
#include "tetriscontroller.h"

#include <functional>

namespace app::ui::scene {

	class LeaveGameData : public SceneData {
	public:
		bool quitApplication = false;
	};

	class LeaveGame : public Scene {
	public:
		LeaveGame(std::shared_ptr<TetrisController> tetrisController, std::function<void()> quitApplication);

		void imGuiUpdate(const DeltaTime& deltaTime) override;
		ImVec2 getSize() const override { return {480.f, 230.f}; }

	private:
		void switchedTo(const SceneData& sceneData) override;

		std::shared_ptr<TetrisController> tetrisController_;
		std::function<void()> quitApplication_;
		bool leaving_ = false;
		bool quitAfterConfirmation_ = false;
	};

}

#endif
