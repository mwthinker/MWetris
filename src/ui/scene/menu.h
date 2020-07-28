#ifndef MWETRIS_UI_SCENE_MENU_H
#define MWETRIS_UI_SCENE_MENU_H

#include "scene.h"
#include "event.h"

namespace mwetris::ui::scene {

	class Menu : public Scene {
	public:
		void imGuiUpdate(const std::chrono::high_resolution_clock::duration& deltaTime) override;

	private:
		void AddMenuButton(const std::string& label, Event event);
	};

}

#endif
