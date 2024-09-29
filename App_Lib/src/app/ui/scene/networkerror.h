#ifndef APP_UI_SCENE_NETWORKERROR_H
#define APP_UI_SCENE_NETWORKERROR_H

#include "scene.h"

#include "types.h"
#include "configuration.h"
#include "ui/imguiextra.h"
#include "game/serialize.h"

namespace app::ui::scene {

	class NetworkError : public Scene {
	public:

		virtual ImVec2 getSize() const {
			return {400, 400};
		}

		void imGuiUpdate(const DeltaTime& deltaTime) override;
	};

}

#endif
