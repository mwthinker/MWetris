#ifndef MWETRIS_UI_TETRISWINDOW_H
#define MWETRIS_UI_TETRISWINDOW_H

#include "scene/scene.h"
#include "scene/statemachine.h"

#include "game/serialize.h"
#include "game/sdldevice.h"
#include "game/computer.h"

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
		void initPreLoop() override;

		void imGuiUpdate(const sdl::DeltaTime& deltaTime) override;

		void imGuiEventUpdate(const SDL_Event& windowEvent) override;

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

		void resumeGame();

		void startNewGame();

		std::vector<game::DevicePtr> getCurrentDevices() const;

		game::DevicePtr findHumanDevice(const std::string& name) const;
	
		sdl::TextureView background_;
		scene::StateMachine sceneStateMachine_;
		
		std::unique_ptr<graphic::GameComponent> gameComponent_;
		std::unique_ptr<game::TetrisGame> game_;
		int nbrHumans_ = 1;
		int nbrAis_ = 0;
		std::vector<game::SdlDevicePtr> devices_;
		std::vector<game::ComputerPtr> computers_;
		mw::signals::ScopedConnections connections_;
		
		bool openPopUp_ = false;
	};

}

#endif
