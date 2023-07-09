#ifndef MWETRIS_UI_SCENE_CUSTOMGAME_H
#define MWETRIS_UI_SCENE_CUSTOMGAME_H

#include "scene.h"

#include "types.h"
#include "configuration.h"
#include "ui/imguiextra.h"
#include "game/serialize.h"

namespace mwetris::ui::scene {

	class CustomGame : public Scene {
	public:
		void imGuiUpdate(const DeltaTime& deltaTime) override;
	};

}

#endif
