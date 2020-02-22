#ifndef DRAWBOARD_H
#define DRAWBOARD_H

#include "../tetrisdata.h"
#include "../logger.h"

#include "tetrisboard.h"
#include "graphic.h"

#include <sdl/imguiauxiliary.h>

#include <vector>

namespace tetris {

	class DrawBoard {
	public:
		DrawBoard();

		Vec2 imGuiToGame(Vec2 pos) const;

		void imGui(float width);

		Vec2 calculateSize(float width) const;

		Vec2 getSize() const;

		void draw(Graphic& graphic);

		void setFont(ImFont* font) {
			font_ = font;
		}

	private:
		Vec2 imGuiSize_;
		
		TetrisBoard tetrisBoard_{10, 24, BlockType::I, BlockType::I};
		Mat4 imGuiToGame_{1};

		Vec2 pos_;
		float width_, height_;
		ImFont* font_{};
	};

}

#endif // DRAWBOARD_H
