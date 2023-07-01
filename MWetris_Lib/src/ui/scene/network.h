#ifndef MWETRIS_UI_SCENE_NETWORK_H
#define MWETRIS_UI_SCENE_NETWORK_H

#include "scene.h"

namespace mwetris::ui::scene {

	class Network : public Scene {
	public:
		void imGuiUpdate(const DeltaTime& deltaTime) override;
	};

}

#endif
