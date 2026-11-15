#ifndef APP_UI_SCENE_GAMELAUNCHER_H
#define APP_UI_SCENE_GAMELAUNCHER_H

#include "scene.h"
#include "tetriscontroller.h"

#include <functional>

namespace app::ui::scene {

	struct GameLauncherActions {
		std::function<void()> startSinglePlayer;
		std::function<void()> createNetworkGame;
		std::function<void()> joinNetworkGame;
		std::function<void()> showHighScores;
		std::function<void()> showPreferences;
		std::function<void()> quit;
	};

	class GameLauncher : public Scene {
	public:
		GameLauncher(std::shared_ptr<TetrisController> tetrisController,
			const GameLauncherActions& actions);

		void imGuiUpdate(const DeltaTime& deltaTime) override;
		ImVec2 getSize() const override { return {500.f, 220.f}; }

	private:
		std::shared_ptr<TetrisController> tetrisController_;
		GameLauncherActions actions_;
	};

}

#endif
