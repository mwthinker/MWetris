#ifndef MWETRIS_GRAPHIC_GAMECOMPONENT_H
#define MWETRIS_GRAPHIC_GAMECOMPONENT_H

#include "imguiboard.h"
#include "types.h"

#include "game/player.h"
#include "game/tetrisgameevent.h"

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

		void gamePause(const game::GamePause& event);

	private:
		std::vector<ImGuiBoard> imguiBoards_;
		
		game::GamePause gamePause_{};
	};

}

#endif
