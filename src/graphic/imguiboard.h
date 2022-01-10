#ifndef MWETRIS_GRAPHIC_IMGUIBOARD_H
#define MWETRIS_GRAPHIC_IMGUIBOARD_H

#include "configuration.h"
#include "game/player.h"
#include "tetrisboard.h"
#include "types.h"

#include <sdl/imguiauxiliary.h>
#include <sdl/sprite.h>

#include <spdlog/spdlog.h>

#include <vector>

namespace mwetris::graphic {

	class ImGuiBoard {
	public:
		explicit ImGuiBoard(game::PlayerPtr playerPtr);

		void draw(float width, float height);

		void callback(tetris::BoardEvent gameEvent, const tetris::TetrisBoard& tetrisBoard);

	private:
		void drawBoard();
		void drawBorder(float width, float height, Color color);

		void drawBlock(const tetris::Block& block, Vec2 pos = {}, bool center = false, Color color = color::White);
		void drawBoardSquares();
		void drawGrid(int width, int height);
		void drawPreviewBlock(tetris::BlockType type, Color color);

		sdl::TextureView getSprite(tetris::BlockType blockType) const;

		float squareSize_ = 0.f;
		float borderSize_ = 2.f;
		
		game::PlayerPtr player_;
		tetris::TetrisBoard tmp_;
		
		float height_ = 0.f;
		sdl::TextureView spriteI_, spriteJ_, spriteL_, spriteO_, spriteS_, spriteT_, spriteZ_;
		sdl::Sprite name_;
	};

}

#endif
