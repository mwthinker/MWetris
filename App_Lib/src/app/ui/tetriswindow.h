#ifndef APP_UI_TETRISWINDOW_H
#define APP_UI_TETRISWINDOW_H

#include "../timerhandler.h"
#include "../configuration.h"
#include "scene/scene.h"
#include "scene/statemachine.h"
#include "scene/addplayer.h"
#include "../game/serialize.h"
#include "../game/devicemanager.h"
#include "../game/tetrisgameevent.h"
#include "../ui/subwindow.h"
#include "../cnetwork/networkevent.h"

#include <network/client.h>

#include <sdl/window.h>

#include <functional>

namespace app {

	class ControlProfiles;
	class TetrisController;
	struct PlayerSlotEvent;
	struct CreateGameEvent;
	struct GameRoomEvent;

	namespace cnetwork {

		class Network;
		struct GameRoomListEvent;

	}

}

namespace app::ui {

	class ControlsPresenter;

	class TetrisWindow : public SubWindow {
	public:
		TetrisWindow(const std::string& windowName, Type type, sdl::Window& window,
			std::shared_ptr<game::DeviceManager> deviceManager,
			std::shared_ptr<ControlProfiles> controlProfiles,
			std::shared_ptr<cnetwork::Network> network,
			std::function<void()> requestRestart
		);

		~TetrisWindow();
	
		void imGuiUpdate(const sdl::DeltaTime& deltaTime) override;
		
		void imGuiEventUpdate(const SDL_Event& windowEvent) override;

		const std::string& getName() const override;

		SubWindow::Type getType() const override;
		void pauseOnFocusLost();
	
	private:
		void onTetrisEvent(const game::GamePause& gamePause);
		void onTetrisEvent(const game::GameOver& gameOver);
		void onTetrisEvent(const PlayerSlotEvent& playerSlotEvent);
		void onTetrisEvent(const app::GameRoomEvent& gameRoomEvent);
		void onTetrisEvent(const cnetwork::GameRoomListEvent& gameRoomListEvent);
		void onTetrisEvent(const game::GameRoomConfigEvent& gameRoomConfigEvent);
		void onTetrisEvent(const cnetwork::NetworkErrorEvent& networkErrorEvent);

		void initPreLoop();
		void refreshSettings();
		game::DevicePtr getSinglePlayerDevice() const;
		void startSinglePlayer();
		void rememberSinglePlayerControls();
		int getCurrentMonitorHz() const;

		void imGuiMainWindow(const sdl::DeltaTime& deltaTime);

		sdl::Window& window_;
		std::shared_ptr<game::DeviceManager> deviceManager_;
		std::shared_ptr<ControlProfiles> controlProfiles_;
		std::shared_ptr<ControlsPresenter> controlsPresenter_;
		std::function<void()> requestRestart_;
		app::TextureView background_;
		scene::StateMachine modalStateMachine_;
		scene::StateMachine mainStateMachine_;

		mw::signals::ScopedConnections connections_;
		std::shared_ptr<TetrisController> tetrisController_;

		TimeHandler timeHandler_;
		std::string pauseMenuText_;
		Configuration::SinglePlayerControls singlePlayerControls_;

		Type type_ = Type::MainWindow;
		std::string windowName_;
	};

}

#endif
