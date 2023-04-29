#ifndef MWETRIS_UI_SCENE_HIGHSCORE_H
#define MWETRIS_UI_SCENE_HIGHSCORE_H

#include "scene.h"

#include "types.h"
#include "configuration.h"
#include "ui/imguiextra.h"
#include "game/serialize.h"

namespace mwetris::ui::scene {

	class HighScore : public Scene {
	public:
		void imGuiUpdate(const DeltaTime& deltaTime) override;

	private:
		void switchedTo() override;
		
		std::vector<game::HighScoreResult> results_;
	};

}

#endif
