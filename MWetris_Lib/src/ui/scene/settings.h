#ifndef MWETRIS_UI_SCENE_SETTINGS_H
#define MWETRIS_UI_SCENE_SETTINGS_H

#include "scene.h"

namespace mwetris::ui::scene {

	class Settings : public Scene {
	public:
		void imGuiUpdate(const DeltaTime& deltaTime) override;
	};

}

#endif
