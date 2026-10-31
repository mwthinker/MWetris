#ifndef APP_UI_SCENE_HIGHSCORE_H
#define APP_UI_SCENE_HIGHSCORE_H

#include "scene.h"

#include "types.h"
#include "configuration.h"
#include "ui/imguiextra.h"
#include "game/serialize.h"
#include "game/serializehighscore.h"

#include <optional>

namespace app::ui::scene {

	struct HighScoreData : public SceneData {
		std::optional<int> highlightedRank;
	};

	class HighScore : public Scene {
	public:
		void imGuiUpdate(const DeltaTime& deltaTime) override;
		ImVec2 getSize() const override { return {620.f, 400.f}; }

	private:
		void switchedTo(const SceneData& sceneData) override;
		
		std::vector<game::HighScoreResult> results_;
		std::optional<int> highlightedRank_;
	};

}

#endif
