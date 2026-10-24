#ifndef APP_UI_SCENE_CONTROLS_H
#define APP_UI_SCENE_CONTROLS_H

#include "scene.h"

#include <memory>

namespace app::ui {

	class ControlsPresenter;

}

namespace app::ui::scene {

	class Controls : public Scene {
	public:
		explicit Controls(std::shared_ptr<ControlsPresenter> controlsPresenter);

		void imGuiUpdate(const DeltaTime& deltaTime) override;
		ImVec2 getSize() const override { return {620.f, 500.f}; }

	private:
		std::shared_ptr<ControlsPresenter> controlsPresenter_;
	};

}

#endif
