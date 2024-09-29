#ifndef MWETRIS_UI_SCENE_RESUME_H
#define MWETRIS_UI_SCENE_RESUME_H

#include "scene.h"
#include "tetriscontroller.h"

#include "game/devicemanager.h"

namespace mwetris::ui::scene {

	class Resume : public Scene {
	public:
		Resume(std::shared_ptr<TetrisController> tetrisController, std::shared_ptr<game::DeviceManager> deviceManager);

		void imGuiUpdate(const DeltaTime& deltaTime) override;

	private:
		std::shared_ptr<TetrisController> tetrisController_;
		std::shared_ptr<game::DeviceManager> deviceManager_;
	};

}

#endif
