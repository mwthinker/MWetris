#ifndef APP_UI_SCENE_HELP_H
#define APP_UI_SCENE_HELP_H

#include "scene.h"

namespace app::ui::scene {

	class HowToPlay : public Scene {
	public:
		void imGuiUpdate(const DeltaTime& deltaTime) override;
		ImVec2 getSize() const override { return {620.f, 470.f}; }
	};

	class KeyboardShortcuts : public Scene {
	public:
		void imGuiUpdate(const DeltaTime& deltaTime) override;
		ImVec2 getSize() const override { return {560.f, 390.f}; }
	};

}

#endif
