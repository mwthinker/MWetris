#ifndef DRAWBOARD_H
#define DRAWBOARD_H

#include "../tetrisdata.h"
#include "../game/player.h"
#include "tetrisboard.h"
#include "graphic.h"

#include <sdl/imguiauxiliary.h>
#include <sdl/sprite.h>

#include <spdlog/spdlog.h>

#include <vector>

namespace tetris {

	class Player;

	class DrawBoard {
	public:
		DrawBoard(Player& player);

		Vec2 getSize() const;

		void draw(Graphic& graphic);

		void callback(BoardEvent gameEvent, const TetrisBoard& tetrisBoard);

	private:
		void drawBlock(Graphic& graphic, const Block& block, Vec2 pos = {}, bool center = false);

		sdl::Sprite getSprite(BlockType blockType) const;

		float squareSize_;
		float borderSize_;
		float infoSize_;
		
		const TetrisBoard& tetrisBoard_;
		mw::signals::Connection connection_;
		
		float width_, height_;
		sdl::Sprite spriteI_, spriteJ_, spriteL_, spriteO_, spriteS_, spriteT_, spriteZ_;
		sdl::Sprite name_;
	};

}

#endif // DRAWBOARD_H
