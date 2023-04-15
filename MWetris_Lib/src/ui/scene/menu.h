#ifndef MWETRIS_UI_SCENE_MENU_H
#define MWETRIS_UI_SCENE_MENU_H

#include "scene.h"
#include "event.h"

namespace mwetris::ui::scene {

	class Menu : public Scene {
	public:
		void imGuiUpdate(const DeltaTime& deltaTime) override;

	private:
		void addMenuButton(const std::string& label, Event event);
	};

}

#endif
