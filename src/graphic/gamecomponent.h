#ifndef MWETRIS_GRAPHIC_GAMECOMPONENT_H
#define MWETRIS_GRAPHIC_GAMECOMPONENT_H

#include "game/player.h"
#include "drawboard.h"
#include "graphic.h"

#include "types.h"
#include "game/tetrisgameevent.h"

#include <sdl/vertexbufferobject.h>
#include <sdl/shaderprogram.h>
#include <sdl/batch.h>

#include <sdl/graphic.h>
#include <mw/signal.h>

#include <entt/entt.hpp>

#include <map>

namespace mwetris::game {

	class TetrisGame;
	class GameData;

}

namespace mwetris::graphic {

	class GameComponent {
	public:
		GameComponent();
		~GameComponent();

		void draw(Graphic& graphic, int width, int height, double deltaTime);

		void initGame(const game::InitGameEvent& event);

	private:
		Mat4 calculateBoardMatrix(int windowWidth, int windowHeight) const;

		//void eventHandler(Event& tetrisGameEvent);
		
		void handleMiddleText(const game::PlayerPtr& player, int lastPostion);

		using DrawBoardPtr = std::unique_ptr<DrawBoard>;
		std::map<game::PlayerPtr, DrawBoardPtr> drawPlayers_;
		
		std::shared_ptr<entt::dispatcher> dispatcher_;

		// Fix time step.
		Uint32 timeStep_{};
		Uint32 accumulator_{};

		// Font related.
		float fontSize_{};
		float borderSize_{};
		entt::scoped_connection initGameEventConnection_;
	};

}

#endif
