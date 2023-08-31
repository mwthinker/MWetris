#ifndef MWETRIS_UI_NETWORKDEBUGWINDOW_H
#define MWETRIS_UI_NETWORKDEBUGWINDOW_H

#include "timerhandler.h"
#include "game/playerslot.h"

#include <sdl/imguiwindow.h>

namespace mwetris::network {

	class DebugClient;

}

namespace mwetris::ui {

	class NetworkDebugWindow {
	public:
		NetworkDebugWindow(std::shared_ptr<network::DebugClient> client);

		void setVisible(bool visible);

		bool isVisible() const;
		
		void imGuiUpdate(const sdl::DeltaTime& deltaTime);

	private:
		void update();

		bool visible_ = false;
		std::shared_ptr<network::DebugClient> debugClient_;
		std::vector<game::PlayerSlot> playerSlots_;
	};

}

#endif
