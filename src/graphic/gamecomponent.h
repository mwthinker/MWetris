#ifndef MWETRIS_GRAPHIC_GAMECOMPONENT_H
#define MWETRIS_GRAPHIC_GAMECOMPONENT_H

#include "game/player.h"

#include "types.h"
#include "game/tetrisgameevent.h"
#include "imguiboard.h"

#include <sdl/graphic.h>

#include <sdl/graphic.h>
#include <mw/signal.h>

#include <set>

namespace mwetris::game {

	class TetrisGame;
	class GameData;

}

namespace mwetris::graphic {

	class GameComponent {
	public:
		GameComponent();
		~GameComponent();

		void draw(int width, int height, double deltaTime);

		void initGame(const game::InitGameEvent& event);

	private:
		//void eventHandler(Event& tetrisGameEvent);
		
		void handleMiddleText(const game::PlayerPtr& player, int lastPostion);

		// Fix time step.
		Uint32 timeStep_{};
		Uint32 accumulator_{};

		// Font related.
		float fontSize_{};
		float borderSize_{};

		std::map<mwetris::game::PlayerPtr, ImGuiBoard> players_;
	};

}

#endif
