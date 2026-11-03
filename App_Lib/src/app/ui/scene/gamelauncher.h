#ifndef APP_UI_SCENE_GAMELAUNCHER_H
#define APP_UI_SCENE_GAMELAUNCHER_H

#include "scene.h"
#include "tetriscontroller.h"

#include <functional>

namespace app::ui::scene {

	class GameLauncher : public Scene {
	public:
		GameLauncher(std::shared_ptr<TetrisController> tetrisController,
			std::function<void()> startSinglePlayer,
			std::function<void()> createNetworkGame, std::function<void()> joinNetworkGame,
			std::function<void()> showHighScores);

		void imGuiUpdate(const DeltaTime& deltaTime) override;
		ImVec2 getSize() const override { return {500.f, 220.f}; }

	private:
		std::shared_ptr<TetrisController> tetrisController_;
		std::function<void()> startSinglePlayer_;
		std::function<void()> createNetworkGame_;
		std::function<void()> joinNetworkGame_;
		std::function<void()> showHighScores_;
	};

}

#endif
