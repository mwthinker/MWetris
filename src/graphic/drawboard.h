#ifndef MWETRIS_GRAPHIC_DRAWBOARD_H
#define MWETRIS_GRAPHIC_DRAWBOARD_H

#include "configuration.h"
#include "game/player.h"
#include "tetrisboard.h"
#include "graphic.h"

#include <sdl/imguiauxiliary.h>
#include <sdl/sprite.h>

#include <spdlog/spdlog.h>

#include <vector>

namespace mwetris::graphic {

	class DrawBoard {
	public:
		DrawBoard(game::Player& player);

		Vec2 getSize() const;

		void draw(Graphic& graphic);

		void callback(tetris::BoardEvent gameEvent, const tetris::TetrisBoard& tetrisBoard);

	private:
		void drawBlock(Graphic& graphic, const tetris::Block& block, Vec2 pos = {}, bool center = false);

		sdl::TextureView getSprite(tetris::BlockType blockType) const;

		float squareSize_;
		float borderSize_;
		float infoSize_;
		
		const tetris::TetrisBoard& tetrisBoard_;
		
		float width_, height_;
		sdl::TextureView spriteI_, spriteJ_, spriteL_, spriteO_, spriteS_, spriteT_, spriteZ_;
		sdl::Sprite name_;
	};

}

#endif
