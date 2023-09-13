#ifndef MWETRIS_UI_NETWORKDEBUGWINDOW_H
#define MWETRIS_UI_NETWORKDEBUGWINDOW_H

#include "timerhandler.h"
#include "game/playerslot.h"

#include <sdl/imguiwindow.h>

#include <network/debugserver.h>

#include "subwindow.h"

namespace mwetris::network {

	class DebugClient;

}

namespace mwetris::graphic {

	class GameComponent;

}

namespace mwetris::ui {

	class NetworkDebugWindow : public SubWindow {
	public:
		explicit NetworkDebugWindow(std::shared_ptr<network::DebugServer> server);
		
		void imGuiUpdate(const sdl::DeltaTime& deltaTime) override;

	private:
		void update();

		std::shared_ptr<network::DebugServer> debugServer_;
		std::vector<game::PlayerSlot> playerSlots_;
		std::unique_ptr<graphic::GameComponent> gameComponent_;
		std::vector<network::ConnectedClient> connectedClients_;
	};

}

#endif
