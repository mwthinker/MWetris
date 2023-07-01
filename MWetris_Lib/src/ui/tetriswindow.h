#ifndef MWETRIS_UI_TETRISWINDOW_H
#define MWETRIS_UI_TETRISWINDOW_H

#include "scene/scene.h"
#include "scene/statemachine.h"

#include "game/serialize.h"

#include <sdl/imguiwindow.h>

namespace mwetris {

	class GameComponent;

}

namespace mwetris::ui::scene {
	class Play;
}

namespace mwetris::ui {

	class TetrisWindow : public sdl::ImGuiWindow {
	public:
		TetrisWindow();

		~TetrisWindow();
	
	private:
		void initPreLoop() override;

		void imGuiUpdate(const sdl::DeltaTime& deltaTime) override;

		void imGuiEventUpdate(const SDL_Event& windowEvent) override;

		template <typename Scene>
		void openPopUp() {
			openPopUp_ = true;
			sceneStateMachine_.switchTo<Scene>();
		}
	
		sdl::TextureView background_;
		scene::StateMachine sceneStateMachine_;
		std::vector<game::HighScoreResult> results_;
		bool openPopUp_ = false;
		std::unique_ptr<scene::Play> play_;
	};

}

#endif
