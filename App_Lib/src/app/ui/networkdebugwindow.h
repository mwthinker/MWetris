#ifndef APP_UI_NETWORKDEBUGWINDOW_H
#define APP_UI_NETWORKDEBUGWINDOW_H

#include "subwindow.h"
#include "../timerhandler.h"
#include "../game/playerslot.h"

#include <network/servercore.h>

#include <sdl/imguiwindow.h>
#include <signal.h>

namespace network {

	class DebugClient;

}

namespace app::graphic {

	class GameComponent;

}

namespace app::ui {

	class NetworkDebugWindow : public SubWindow {
	public:
		explicit NetworkDebugWindow(std::shared_ptr<network::ServerCore> server);
		~NetworkDebugWindow() override;
		
		void imGuiUpdate(const sdl::DeltaTime& deltaTime) override;

		const std::string& getName() const override {
			return name_;
		}

		SubWindow::Type getType() const override {
			return SubWindow::Type::SecondaryWindow;
		}

	private:
		void update();

		std::shared_ptr<network::ServerCore> server_;
		std::vector<network::Slot> playerSlots_;
		std::unique_ptr<graphic::GameComponent> gameComponent_;
		std::vector<network::ConnectedClient> connectedClients_;
		std::string name_ = "NetworkDebugWindow";
		mw::signals::ScopedConnections connections_;
	};

}

#endif
