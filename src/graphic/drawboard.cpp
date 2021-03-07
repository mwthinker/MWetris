#include "drawboard.h"
#include "tetrisdata.h"
#include "tetrisboard.h"
#include "graphic.h"
#include "game/player.h"

#include <sdl/imguiauxiliary.h>

#include <spdlog/spdlog.h>

#include <vector>

namespace mwetris::graphic {

	namespace {

		Vec2 calculateCenterOfMass(const tetris::Block& block) {
			Vec2 pos{};
			for (const auto& sq : block) {
				pos += Vec2{sq.column, sq.row};
			}
			return 1.f / block.getSize() * pos;
		}

		void addRectangle(Graphic& graphic, float x, float y, float w, float h, Color color) {
			graphic.addRectangle({x, y}, {w, h}, color);
		}

		void addSquare(Graphic& graphic, float x, float y, float size, Color color) {
			graphic.addRectangle({x, y}, {size, size}, color);
		}

		void addSquare(Graphic& graphic, float x, float y, float size, const sdl::Sprite& sprite) {
			graphic.addRectangleImage({x, y}, {size, size}, sprite.getTextureView());
		}

		void addText(Graphic& graphic, float x, float y, const sdl::Sprite& text) {
			auto view = text.getTextureView();
			graphic.addRectangleImage(Vec2{x, y} + view.getPosition(), 0.5f * text.getSize(), view);
		}

		constexpr float middleDistance = 5.f;
		constexpr float rightDistance = 5.f;

	}

	DrawBoard::DrawBoard(game::Player& player)
		: tetrisBoard_{player.getTetrisBoard()} {

		spriteZ_ = TetrisData::getInstance().getSprite(tetris::BlockType::Z);
		spriteS_ = TetrisData::getInstance().getSprite(tetris::BlockType::S);
		spriteJ_ = TetrisData::getInstance().getSprite(tetris::BlockType::J);
		spriteI_ = TetrisData::getInstance().getSprite(tetris::BlockType::I);
		spriteL_ = TetrisData::getInstance().getSprite(tetris::BlockType::L);
		spriteT_ = TetrisData::getInstance().getSprite(tetris::BlockType::T);
		spriteO_ = TetrisData::getInstance().getSprite(tetris::BlockType::O);

		squareSize_ = TetrisData::getInstance().getTetrisSquareSize();
		borderSize_ = TetrisData::getInstance().getTetrisBorderSize();
		infoSize_ = squareSize_ * 5.f;

		name_ = sdl::Sprite{"Test", TetrisData::getInstance().getDefaultFont(50)};
		name_.bindTexture();

		width_ = squareSize_ * tetrisBoard_.getColumns() + infoSize_ + borderSize_ * 2 + middleDistance + rightDistance;
		height_ = squareSize_ * (tetrisBoard_.getRows() - 2) + borderSize_ * 2;
	}

	Vec2 DrawBoard::getSize() const {
		return {width_, height_};
	}

	void DrawBoard::drawBlock(Graphic& graphic, const tetris::Block& block, Vec2 pos, bool center) {
		Vec2 delta{};
		if (center) {
			delta = (-Vec2{0.5f, 0.5f} - calculateCenterOfMass(block)) * squareSize_;
		}

		for (const auto& sq : block) {
			const int columns = tetrisBoard_.getColumns();
			auto x = pos.x + borderSize_ + squareSize_ * sq.column + delta.x;
			auto y = pos.y + borderSize_ + squareSize_ * sq.row + delta.y;

			if (sq.row < tetrisBoard_.getRows() - 2) {
				addSquare(graphic,
					x, y,
					squareSize_,
					getSprite(block.getBlockType()));
			}
		}
	}

	void DrawBoard::draw(Graphic& graphic) {
		const int columns = tetrisBoard_.getColumns();
		const int rows = tetrisBoard_.getRows();
		
		const float boardWidth = squareSize_ * columns;
		//restart(player); // Must be called after variables are defined.

		// Draw the player area.
		float x = borderSize_;
		float y = borderSize_;
		addRectangle(graphic,
			x, y,
			boardWidth + infoSize_ + middleDistance + rightDistance, squareSize_ * (rows - 2),
			TetrisData::getInstance().getPlayerAreaColor());

		// Draw the outer square.
		x = borderSize_;
		y = borderSize_;
		addRectangle(graphic,
			x, y,
			squareSize_ * columns, squareSize_ * (rows - 2),
			TetrisData::getInstance().getOuterSquareColor());

		// Draw the inner squares.
		for (int row = 0; row < rows - 2; ++row) {
			for (int column = 0; column < columns; ++column) {
				x = borderSize_ + squareSize_ * column + squareSize_ * 0.1f;
				y = borderSize_ + squareSize_ * row + squareSize_ * 0.1f;
				addRectangle(graphic,
					x, y,
					squareSize_ * 0.8f, squareSize_ * 0.8f,
					TetrisData::getInstance().getInnerSquareColor());
			}
		}

		// Draw the block start area.
		x = borderSize_;
		y = borderSize_ + squareSize_ * (rows - 4);
		addRectangle(graphic,
			x, y,
			squareSize_ * columns, squareSize_ * 2,
			TetrisData::getInstance().getStartAreaColor());
			

		// Draw the preview block area.
		x = borderSize_ + boardWidth + middleDistance;
		y = borderSize_ + squareSize_ * (rows - 4) - (squareSize_ * 5 + middleDistance);
		addSquare(graphic,
			x, y,
			infoSize_,
			TetrisData::getInstance().getStartAreaColor());

		drawBlock(graphic, tetris::Block{tetrisBoard_.getNextBlockType(), 0, 0}, Vec2{x, y} + squareSize_ * 2.5f, true);

		const Color borderColor = TetrisData::getInstance().getBorderColor();

		// Add border.
		// Left-up corner.
		x = 0.f;
		y = height_ - borderSize_;
		addSquare(graphic,
			x, y,
			borderSize_,
			borderColor);

		// Right-up corner.
		x = width_ - borderSize_;
		y = height_ - borderSize_;
		addSquare(graphic,
			x, y,
			borderSize_,
			borderColor);

		// Left-down corner.
		x = 0.f;
		y = 0.f;
		addSquare(graphic,
			x, y,
			borderSize_,
			borderColor);

		// Right-down corner.
		x = width_ - borderSize_;
		y = 0.f;
		addSquare(graphic,
			x, y,
			borderSize_,
			borderColor);

		// Up.
		x = borderSize_;
		y = height_ - borderSize_;
		addRectangle(graphic,
			x, y,
			width_ - 2 * borderSize_, borderSize_,
			borderColor);

		// Down.
		x = borderSize_;
		y = 0.f;
		addRectangle(graphic,
			x, y,
			width_ - 2 * borderSize_, borderSize_,
			borderColor);

		// Left.
		x = 0.f;
		y = borderSize_;
		addRectangle(graphic,
			x, y,
			borderSize_, height_ - 2 * borderSize_,
			borderColor);

		// Right.
		x = width_ - borderSize_;
		y = borderSize_;
		addRectangle(graphic,
			x, y,
			borderSize_, height_ - 2 * borderSize_,
			borderColor);

		drawBlock(graphic, tetrisBoard_.getBlock());

		for (int i = 0; i < tetrisBoard_.getRows() - 2; ++i) {
			for (int j = 0; j < tetrisBoard_.getColumns(); ++j) {
				x = borderSize_ + squareSize_ * j;
				y = borderSize_ + squareSize_ * i;

				auto blockType = tetrisBoard_.getBlockType(j, i);
				if (blockType != tetris::BlockType::Empty
					&& blockType != tetris::BlockType::Wall) {
					
					addSquare(graphic,
						x, y,
						squareSize_,
						getSprite(blockType));
				}
			}
		}

		addText(graphic, 100, 100, name_);
		
	}

	void DrawBoard::callback(tetris::BoardEvent gameEvent, const tetris::TetrisBoard& tetrisBoard) {
		/*
		for (auto& row : rows_) {
			row->handleEvent(gameEvent, tetrisBoard);
		}
		rows_.remove_if([&](const DrawRowPtr& row) {
			if (!row->isAlive()) {
				freeRows_.push_front(row);
				return true;
			}
			return false;
		});
		switch (gameEvent) {
			case BoardEvent::GAME_OVER:
				break;
			case BoardEvent::BLOCK_COLLISION:
				break;
			case BoardEvent::RESTARTED:
				break;
			case BoardEvent::EXTERNAL_ROWS_ADDED:
			{
				int rows = tetrisBoard.getNbrExternalRowsAdded();
				for (int row = 0; row < rows; ++row) {
					addDrawRowBottom(tetrisBoard, rows - row - 1);
				}
				int highestRow = tetrisBoard.getBoardVector().size() / tetrisBoard.getColumns();
				assert(rows_.size() - highestRow >= 0); // Something is wrong. Should not be posssible.
				for (int i = 0; i < (int) rows_.size() - highestRow; ++i) { // Remove unneeded empty rows at the top.
					rows_.pop_back();
				}
			}
			break;
			case BoardEvent::NEXT_BLOCK_UPDATED:
				nextBlock_.update(Block(tetrisBoard.getNextBlockType(), 0, 0));
				break;
			case BoardEvent::CURRENT_BLOCK_UPDATED:
				// Fall through!
			case BoardEvent::PLAYER_MOVES_BLOCK_ROTATE:
				// Fall through!		
			case BoardEvent::PLAYER_MOVES_BLOCK_LEFT:
				// Fall through!
			case BoardEvent::PLAYER_MOVES_BLOCK_RIGHT:
				currentBlock_.update(tetrisBoard.getBlock());
				{
					TetrisBoard board = tetrisBoard;
					board.update(Move::DOWN_GROUND);
					downBlock_.update(board.getBlock());
				}
				break;
			case BoardEvent::PLAYER_MOVES_BLOCK_DOWN_GROUND:
				blockDownGround_ = true;
				latestBlockDownGround_ = tetrisBoard.getBlock();
				break;
			case BoardEvent::PLAYER_MOVES_BLOCK_DOWN:
				if (blockDownGround_) {
					currentBlock_.updateDown(tetrisBoard.getBlock());
					blockDownGround_ = false;
				}
				// Fall through!
			case BoardEvent::GRAVITY_MOVES_BLOCK:
				currentBlock_.update(tetrisBoard.getBlock());
				break;
			case BoardEvent::ROW_TO_BE_REMOVED:
				textClearedRows_.update("Rows " + std::to_string(tetrisBoard.getRemovedRows()));
				break;
			case BoardEvent::ONE_ROW_REMOVED:
				addDrawRowAtTheTop(tetrisBoard, 1);
				break;
			case BoardEvent::TWO_ROW_REMOVED:
				addDrawRowAtTheTop(tetrisBoard, 2);
				break;
			case BoardEvent::THREE_ROW_REMOVED:
				addDrawRowAtTheTop(tetrisBoard, 3);
				break;
			case BoardEvent::FOUR_ROW_REMOVED:
				addDrawRowAtTheTop(tetrisBoard, 4);
				break;
		}
		*/
	}

	sdl::Sprite DrawBoard::getSprite(tetris::BlockType blockType) const {
		switch (blockType) {
			case tetris::BlockType::I:
				return spriteI_;
			case tetris::BlockType::J:
				return spriteJ_;
			case tetris::BlockType::L:
				return spriteL_;
			case tetris::BlockType::O:
				return spriteO_;
			case tetris::BlockType::S:
				return spriteS_;
			case tetris::BlockType::T:
				return spriteT_;
			case tetris::BlockType::Z:
				return spriteZ_;
		}
		return spriteI_;
	}

}
