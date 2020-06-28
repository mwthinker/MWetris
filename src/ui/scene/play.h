#ifndef MWETRIS_UI_SCENE_PLAY_H
#define MWETRIS_UI_SCENE_PLAY_H

#include "scene.h"

#include "../../types.h"
#include "../../tetrisdata.h"
#include "../imguiextra.h"

namespace tetris::ui::scene {

	class Play : public Scene {
	public:
		void draw(const std::chrono::high_resolution_clock::duration& deltaTime) override;

		void imGuiUpdate(const std::chrono::high_resolution_clock::duration& deltaTime) override;

	private:
		
	};

}

#endif
