#ifndef MWETRIS_GRAPHIC_GRAPHICBOARD_H
#define MWETRIS_GRAPHIC_GRAPHICBOARD_H

#include "../game/player.h"
#include "drawrow.h"
#include "tetrisboard.h"
#include "../types.h"

#include <sdl/font.h>
#include <sdl/sprite.h>
#include <sdl/batch.h>
#include <sdl/vertex.h>
#include <mw/signal.h>

#include <random>
#include <string>
#include <list>

namespace tetris::graphic {

	class GameGraphic {
	public:
		GameGraphic() = default;

		~GameGraphic();

		//void restart(BoardBatch& boardBatch, Player& player, float x, float y, bool showPoints);

		void restart(game::Player& player);

		float getWidth() const {
			return width_;
		}

		float getHeight() const {
			return height_;
		}

		//void update(float deltaTime, BoardBatch& dynamicBoardBatch);

		void callback(BoardEvent gameEvent, const TetrisBoard& tetrisBoard);

	private:
		//void initStaticBackground(BoardBatch& boardBatch, float lowX, float lowY, Player& player);

		void addDrawRowAtTheTop(const TetrisBoard& tetrisBoard, int nbr);

		void addEmptyRowTop(const TetrisBoard& tetrisBoard);

		void addDrawRowBottom(const TetrisBoard& tetrisBoard, int row);

		std::list<DrawRowPtr> rows_;
		std::list<DrawRowPtr> freeRows_;
		
		sdl::SubBatch<sdl::Vertex> backGroundBatch_;
		Block latestBlockDownGround_;
		bool blockDownGround_;
		bool showDownBlock_;

		mw::signals::Connection connection_;
		float width_, height_;
		float lowX_, lowY_;
	};

}

#endif
