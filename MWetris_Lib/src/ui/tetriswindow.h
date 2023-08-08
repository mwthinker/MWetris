#ifndef MWETRIS_UI_TETRISWINDOW_H
#define MWETRIS_UI_TETRISWINDOW_H

#include "timerhandler.h"

#include "scene/scene.h"
#include "scene/statemachine.h"
#include "scene/addplayer.h"

#include "game/serialize.h"
#include "game/devicemanager.h"

#include <sdl/imguiwindow.h>

namespace mwetris {

	class GameComponent;

}

namespace mwetris::graphic {

	class GameComponent;

}

namespace mwetris::game {

	class TetrisGame;

}

namespace mwetris::ui {

	class TetrisWindow : public sdl::ImGuiWindow {
	public:
		TetrisWindow();

		~TetrisWindow();
	
	private:
		int getCurrentMonitorHz() const;

		void initPreLoop() override;

		void eventUpdate(const SDL_Event& windowEvent) override;

		void imGuiUpdate(const sdl::DeltaTime& deltaTime) override;

		void imGuiEventUpdate(const SDL_Event& windowEvent) override;

		void imGuiCustomGame(int windowWidth, int windowHeight, double deltaTime);

		template <typename Scene>
		void openPopUp() {
			scene::SceneData data;
			openPopUp<Scene>(data);
		}

		template <typename Scene>
		void openPopUp(const scene::SceneData& data) {
			openPopUp_ = true;
			sceneStateMachine_.switchTo<Scene>(data);
		}

		void imGuiMainWindow(const sdl::DeltaTime& deltaTime);

		void startNewGame();

		sdl::TextureView background_;
		scene::StateMachine sceneStateMachine_;
		
		std::unique_ptr<graphic::GameComponent> gameComponent_;
		std::shared_ptr<game::TetrisGame> game_;
		mw::signals::ScopedConnections connections_;

		bool openPopUp_ = false;
		std::shared_ptr<game::DeviceManager> deviceManager_;

		TimeHandler timeHandler_;
		std::string pauseMenuText_;

		bool customGame = true;
		std::vector<scene::PlayerSlot> playerSlots_;
	};

}

#endif
