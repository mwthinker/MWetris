#ifndef APP_UI_SCENE_PLAY_H
#define APP_UI_SCENE_PLAY_H

#include "scene.h"
#include "resume.h"
#include "tetriscontroller.h"

#include "game/devicemanager.h"

namespace app::ui::scene {

	class Play : public Scene {
	public:
		explicit Play(std::shared_ptr<TetrisController> tetrisController);

		void imGuiUpdate(const DeltaTime& deltaTime) override;

	private:
		std::shared_ptr<TetrisController> tetrisController_;
	};

}

#endif
