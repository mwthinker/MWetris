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

		void gamePause(const game::GamePause& event);
		
		void countDown(const game::CountDown& countDown);

	private:
		std::map<mwetris::game::PlayerPtr, ImGuiBoard> players_;
		bool paused_ = false;
		int timeLeft_ = -1;
	};

}

#endif
