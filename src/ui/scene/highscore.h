#ifndef MWETRIS_UI_SCENE_HIGHSCORE_H
#define MWETRIS_UI_SCENE_HIGHSCORE_H

#include "scene.h"

#include "types.h"
#include "tetrisdata.h"
#include "ui/imguiextra.h"

namespace tetris::ui::scene {

	class HighScore : public Scene {
	public:
		void imGuiUpdate(const std::chrono::high_resolution_clock::duration& deltaTime) override;

	private:
		
	};

}

#endif