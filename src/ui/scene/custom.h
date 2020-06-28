#ifndef MWETRIS_UI_SCENE_CUSTOM_H
#define MWETRIS_UI_SCENE_CUSTOM_H

#include "scene.h"

#include "../../types.h"
#include "../../tetrisdata.h"
#include "../imguiextra.h"

namespace tetris::ui::scene {

	class Custom : public Scene {
	public:
		void imGuiUpdate(const std::chrono::high_resolution_clock::duration& deltaTime) override;

	private:
		
	};

}

#endif
