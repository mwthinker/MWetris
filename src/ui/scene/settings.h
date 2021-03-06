#ifndef MWETRIS_UI_SCENE_SETTINGS_H
#define MWETRIS_UI_SCENE_SETTINGS_H

#include "scene.h"

#include "types.h"
#include "configuration.h"
#include "ui/imguiextra.h"

namespace mwetris::ui::scene {

	class Settings : public Scene {
	public:
		void imGuiUpdate(const DeltaTime& deltaTime) override;

	private:
		
	};

}

#endif
