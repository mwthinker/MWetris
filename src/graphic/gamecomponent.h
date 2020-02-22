#ifndef GAMECOMPONENT_H
#define GAMECOMPONENT_H

#include "gamegraphic.h"
#include "../game/player.h"

#include "../types.h"

#include <sdl/vertexbufferobject.h>
#include <sdl/shaderprogram.h>
#include <sdl/batch.h>
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

		void draw(int width, int height, double deltaTime);

	private:
		void initGame(std::vector<PlayerPtr>& player);

		void eventHandler(TetrisGameEvent& tetrisGameEvent);
		
		void validate();

		void handleMiddleText(const PlayerPtr& player, int lastPostion);

		std::map<PlayerPtr, GameGraphic> graphicPlayers_;
		//BoardShaderPtr boardShader_;

		//std::shared_ptr<BoardBatch> staticBoardBatch_;
		//std::shared_ptr<BoardBatch> dynamicBoardBatch_;

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
