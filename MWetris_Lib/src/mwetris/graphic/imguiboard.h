#ifndef MWETRIS_GRAPHIC_IMGUIBOARD_H
#define MWETRIS_GRAPHIC_IMGUIBOARD_H

#include "types.h"
#include "game/player.h"

#include <tetris/tetrisboard.h>

#include <sdl/sprite.h>

#include <vector>

namespace mwetris::graphic {

	class ImGuiBoard {
	public:
		explicit ImGuiBoard(game::PlayerPtr player);

		void draw(float width, float height, double deltaTime);

		const game::Player& getPlayer() const;

		ImGuiBoard(const ImGuiBoard& imGuiBoard);
		ImGuiBoard& operator=(const ImGuiBoard& imGuiBoard);

	private:
		void handlePlayerBoardEvent(const game::PlayerBoardEvent& playerBoardEvent);

		void handleGameBoardEvent(tetris::BoardEvent gameEvent, int nbr);

		void drawBoard(double deltaTime);
		void drawBorder(float width, float height, Color color);

		void drawBlock(const tetris::Block& block, Vec2 pos = {}, bool center = false, Color color = color::White);
		void drawBoardSquares(double deltaTime);
		void drawGrid(int columns, int rows);
		void drawPreviewBlock(tetris::BlockType type, Color color);

		sdl::TextureView getSprite(tetris::BlockType blockType) const;

		float squareSize_ = 0.f;
		float borderSize_ = 2.f;
		
		game::PlayerPtr player_;
		
		std::vector<float> rows_;
		float height_ = 0.f;
		sdl::TextureView spriteI_, spriteJ_, spriteL_, spriteO_, spriteS_, spriteT_, spriteZ_;
		mw::signals::ScopedConnections connections_;
	};

}

#endif
