#ifndef MWETRIS_UI_TETRISWINDOW_H
#define MWETRIS_UI_TETRISWINDOW_H

#include "../timerhandler.h"
#include "scene/scene.h"
#include "scene/statemachine.h"
#include "scene/addplayer.h"
#include "../game/serialize.h"
#include "../game/devicemanager.h"
#include "../game/tetrisgameevent.h"
#include "../network/client.h"
#include "../network/networkevent.h"
#include "../ui/subwindow.h"

#include <sdl/imguiwindow.h>

namespace mwetris {

	class TetrisController;
	struct PlayerSlotEvent;
	struct CreateGameEvent;
	struct GameRoomEvent;

	namespace network {

		class Network;
		struct GameRoomListEvent;

	}

}

namespace mwetris::ui {

	class TetrisWindow : public SubWindow {
	public:
		TetrisWindow(const std::string& windowName, Type type, sdl::Window& window,
			std::shared_ptr<game::DeviceManager> deviceManager,
			std::shared_ptr<network::Network> network
		);

		~TetrisWindow();
	
		void imGuiUpdate(const sdl::DeltaTime& deltaTime) override;
		
		void imGuiEventUpdate(const SDL_Event& windowEvent) override;

		const std::string& getName() const override;

		SubWindow::Type getType() const override;
	
	private:
		void onTetrisEvent(const game::GamePause& gamePause);
		void onTetrisEvent(const game::GameOver& gameOver);
		void onTetrisEvent(const PlayerSlotEvent& playerSlotEvent);
		void onTetrisEvent(const mwetris::GameRoomEvent& gameRoomEvent);
		void onTetrisEvent(const CreateGameEvent& createGameEvent);
		void onTetrisEvent(const network::GameRoomListEvent& gameRoomListEvent);
		void onTetrisEvent(const game::GameRoomConfigEvent& gameRoomConfigEvent);

		void initPreLoop();
		int getCurrentMonitorHz() const;

		template <typename Scene>
		void openPopUp() {
			scene::SceneData data;
			openPopUp<Scene>(data);
		}

		template <typename Scene>
		void openPopUp(const scene::SceneData& data) {
			openPopUp_ = true;
			modalStateMachine_.switchTo<Scene>(data);
		}

		void imGuiMainWindow(const sdl::DeltaTime& deltaTime);

		void startNewGame();

		sdl::Window& window_;
		std::shared_ptr<game::DeviceManager> deviceManager_;
		sdl::TextureView background_;
		scene::StateMachine modalStateMachine_;
		scene::StateMachine mainStateMachine_;

		mw::signals::ScopedConnections connections_;
		std::shared_ptr<TetrisController> tetrisController_;

		bool openPopUp_ = false;

		TimeHandler timeHandler_;
		std::string pauseMenuText_;

		Type type_ = Type::MainWindow;
		std::string windowName_;
		std::string popUpId_;
	};

}

#endif
