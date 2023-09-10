#ifndef MWETRIS_UI_NETWORKDEBUGWINDOW_H
#define MWETRIS_UI_NETWORKDEBUGWINDOW_H

#include "timerhandler.h"
#include "game/playerslot.h"

#include <sdl/imguiwindow.h>

namespace mwetris::network {

	class DebugClient;

}

namespace mwetris::graphic {

	class GameComponent;

}

namespace mwetris::ui {

	class SubWindow {
	public:
		virtual ~SubWindow() = default;

		virtual void imGuiUpdate(const sdl::DeltaTime& deltaTime) = 0;
	};

	class NetworkDebugWindow : public SubWindow {
	public:
		explicit NetworkDebugWindow(std::shared_ptr<network::DebugClient> client);
		
		void imGuiUpdate(const sdl::DeltaTime& deltaTime) override;

	private:
		void update();

		std::shared_ptr<network::DebugClient> debugClient_;
		std::vector<game::PlayerSlot> playerSlots_;
		std::unique_ptr<graphic::GameComponent> gameComponent_;
	};

}

#endif
