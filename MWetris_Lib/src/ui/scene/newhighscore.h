#ifndef MWETRIS_UI_SCENE_NEWHIGHSCORE_H
#define MWETRIS_UI_SCENE_NEWHIGHSCORE_H

#include "scene.h"

#include "types.h"
#include "configuration.h"
#include "ui/imguiextra.h"
#include "game/serialize.h"

#include <functional>

namespace mwetris::ui::scene {

	struct NewHighScoreData : public scene::SceneData {
		std::string name;
		int points;
		int clearedRows;
		int level;
	};

	class NewHighScore : public Scene {
	public:
		NewHighScore(std::function<void()> closeFunction) {
			closeFunction_ = closeFunction;
		}

		void imGuiUpdate(const DeltaTime& deltaTime) override;

	private:
		void save();

		void switchedTo(const SceneData& sceneData) override;
		
		NewHighScoreData data_{};
		std::function<void()> closeFunction_;
	};

}

#endif
