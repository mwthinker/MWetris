#ifndef MWETRIS_UI_SCENE_NEWHIGHSCORE_H
#define MWETRIS_UI_SCENE_NEWHIGHSCORE_H

#include "scene.h"

#include "types.h"
#include "configuration.h"
#include "ui/imguiextra.h"
#include "game/serialize.h"

namespace mwetris::ui::scene {

	class NewHighScore : public Scene {
	public:
		void imGuiUpdate(const DeltaTime& deltaTime) override;

	private:
		void switchedTo(SceneData& sceneData) override;
		
		std::string name_;
	};

}

#endif
