#ifndef APP_UI_SCENE_SETTINGS_H
#define APP_UI_SCENE_SETTINGS_H

#include "scene.h"

namespace app::ui::scene {

	class Settings : public Scene {
	public:
		void imGuiUpdate(const DeltaTime& deltaTime) override;
	};

}

#endif
