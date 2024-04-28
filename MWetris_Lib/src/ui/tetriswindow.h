#ifndef MWETRIS_UI_TETRISWINDOW_H
#define MWETRIS_UI_TETRISWINDOW_H

#include "timerhandler.h"

#include "networkdebugwindow.h"

#include "scene/scene.h"
#include "scene/statemachine.h"
#include "scene/addplayer.h"

#include "game/serialize.h"
#include "game/devicemanager.h"

#include "network/network.h"
#include "network/client.h"

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

	class TetrisWindow : public SubWindow {
	public:
		TetrisWindow(const std::string& windowName, Type type, sdl::Window& window,
			std::shared_ptr<game::DeviceManager> deviceManager,
			std::shared_ptr<network::Client> client
		);

		~TetrisWindow();
	
		void imGuiUpdate(const sdl::DeltaTime& deltaTime) override;
		
		void imGuiEventUpdate(const SDL_Event& windowEvent) override;

		const std::string& getName() const override;

		SubWindow::Type getType() const override;
	
	private:
		void initPreLoop();
		int getCurrentMonitorHz() const;

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

		sdl::Window& window_;
		std::shared_ptr<game::DeviceManager> deviceManager_;
		std::shared_ptr<network::Network> network_;
		std::shared_ptr<network::Client> client_;
		sdl::TextureView background_;
		scene::StateMachine sceneStateMachine_;
		
		std::unique_ptr<graphic::GameComponent> gameComponent_;
		std::shared_ptr<game::TetrisGame> game_;
		mw::signals::ScopedConnections connections_;

		bool openPopUp_ = false;

		TimeHandler timeHandler_;
		std::string pauseMenuText_;

		bool customGame_ = false;
		std::vector<game::PlayerSlot> playerSlots_;
		Type type_ = Type::MainWindow;
		std::string gameRoomUuid_;
		std::string windowName_;
	};

}

#endif
