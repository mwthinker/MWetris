#ifndef GAMECOMPONENT_H
#define GAMECOMPONENT_H

#include "gamegraphic.h"
#include "../game/player.h"
#include "drawboard.h"
#include "graphic.h"

#include "../types.h"

#include <sdl/vertexbufferobject.h>
#include <sdl/shaderprogram.h>
#include <sdl/batch.h>

#include <sdl/graphic.h>
#include <mw/signal.h>

#include <map>

namespace tetris {

	class TetrisGame;
	class GameData;
	class TetrisGameEvent;

	class GameComponent {
	public:
		GameComponent(TetrisGame& tetrisGame);
		~GameComponent();

		void draw(Graphic& graphic, int width, int height, double deltaTime);

	private:
		void initGame(const std::vector<PlayerPtr>& player);

		void eventHandler(TetrisGameEvent& tetrisGameEvent);
		
		void validate();

		void handleMiddleText(const PlayerPtr& player, int lastPostion);

		using DrawBoardPtr = std::unique_ptr<DrawBoard>;
		std::map<PlayerPtr, DrawBoardPtr> drawPlayers_;

		TetrisGame& tetrisGame_;

		mw::signals::Connection eventConnection_;

		// Fix time step.
		Uint32 timeStep_;
		Uint32 accumulator_;

		// Updated in initGame().
		Mat4 matrix_;
		Mat4 model_;
		Mat4 projMatrix_;
		bool updateMatrix_;

		// Font related.
		sdl::Text middleText_;
		float fontSize_{};
		float dx_{}, dy_{};
		float scale_{};
		float borderSize_{};
	};

}

#endif // GAMECOMPONENT_H
