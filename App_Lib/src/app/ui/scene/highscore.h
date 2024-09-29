#ifndef APP_UI_SCENE_HIGHSCORE_H
#define APP_UI_SCENE_HIGHSCORE_H

#include "scene.h"

#include "types.h"
#include "configuration.h"
#include "ui/imguiextra.h"
#include "game/serialize.h"
#include "game/serializehighscore.h"

namespace app::ui::scene {

	class HighScore : public Scene {
	public:
		void imGuiUpdate(const DeltaTime& deltaTime) override;

	private:
		void switchedTo(const SceneData& sceneData) override;
		
		std::vector<game::HighScoreResult> results_;
	};

}

#endif
