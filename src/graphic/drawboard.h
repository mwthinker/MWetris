#ifndef DRAWBOARD_H
#define DRAWBOARD_H

#include "../tetrisdata.h"
#include "../logger.h"
#include "../game/player.h"

#include "tetrisboard.h"
#include "graphic.h"

#include <sdl/imguiauxiliary.h>
#include <sdl/sprite.h>

#include <vector>

namespace tetris {

	class Player;

	class DrawBoard {
	public:
		DrawBoard(Player& player);

		Vec2 imGuiToGame(Vec2 pos) const;

		Vec2 getSize() const;

		void imGui(float width);

		void draw(Graphic& graphic);

		void callback(BoardEvent gameEvent, const TetrisBoard& tetrisBoard);

	private:
		void drawBlock(Graphic& graphic, const Block& block, Vec2 pos = {}, bool center = false);

		sdl::Sprite getSprite(BlockType blockType) const;

		Vec2 imGuiSize_;
		float squareSize_;
		float borderSize_;
		float infoSize_;
		
		const TetrisBoard& tetrisBoard_;
		Mat4 imGuiToGame_{1};
		mw::signals::Connection connection_;

		Vec2 pos_;
		float width_, height_;
		sdl::Sprite spriteI_, spriteJ_, spriteL_, spriteO_, spriteS_, spriteT_, spriteZ_;
	};

}

#endif // DRAWBOARD_H
