#include "drawboard.h"
#include "../tetrisdata.h"
#include "../logger.h"

#include "tetrisboard.h"
#include "graphic.h"

#include <sdl/imguiauxiliary.h>

#include <vector>

namespace tetris {

	namespace {

		void addRectangle(Graphic& graphic, float x, float y, float w, float h, Color color) {
			graphic.addRectangle({x, y}, {w, h}, color);
		}

		void addSquare(Graphic& graphic, float x, float y, float size, Color color) {
			graphic.addRectangle({x, y}, {size, size}, color);
		}

		constexpr float middleDistance = 5.f;
		constexpr float rightDistance = 5.f;

	}

	DrawBoard::DrawBoard() {
	}

	Vec2 DrawBoard::imGuiToGame(Vec2 pos) const {
		return {pos.x, ImGui::GetIO().DisplaySize.y - pos.y + TetrisData::getInstance().getTetrisBorderSize() + 1};
	}

	void DrawBoard::imGui(float width) {
		const float squareSize = TetrisData::getInstance().getTetrisSquareSize();
		const float borderSize = TetrisData::getInstance().getTetrisBorderSize();
		
		const float infoSize = squareSize * 5.f;
		width_ = squareSize * tetrisBoard_.getColumns() + infoSize + borderSize * 2 + middleDistance + rightDistance;
		height_ = squareSize * (tetrisBoard_.getRows() - 2) + borderSize * 2;

		imGuiSize_ = {width, height_ / width_ * width};
		ImGui::ChildWindow("", imGuiSize_, [&]() {
			ImGui::Columns(2, nullptr, false);
			const float boardSize = squareSize * tetrisBoard_.getColumns() + borderSize;
			ImGui::SetColumnWidth(-1, boardSize * imGuiSize_.x / width_);
			//ImGui::SetColumnOffset
			
			//ImGui::Button("HAHA", {boardSize * imGuiSize_.x / width_, imGuiSize_.y});
			ImGui::NextColumn();
			ImGui::Dummy({4,  imGuiSize_.y * 0.5f});
			ImGui::PushFont(font_);
			ImGui::Text("Marcus");
			
			ImGui::Text("Points");
			ImGui::Text("Level");
			ImGui::Text("Rows");

			ImGui::PopFont();
			//ImGui::SameLine(50);
			//ImGui::TextColored(tetris::TetrisData::getInstance().getLabelTextColor(), "Marcus");
			//ImGui::Button("ASDAS", {50,50});
		});
		
		
		pos_ = imGuiToGame(ImGui::GetCursorScreenPos());
	}

	Vec2 DrawBoard::calculateSize(float width) const {
		const float squareSize = TetrisData::getInstance().getTetrisSquareSize();
		const float borderSize = TetrisData::getInstance().getTetrisBorderSize();

		const float infoSize = squareSize * 5.f;
		float w = squareSize * tetrisBoard_.getColumns() + infoSize + borderSize * 2 + middleDistance + rightDistance;
		float h = squareSize * (tetrisBoard_.getRows() - 2) + borderSize * 2;
		return {w, h};
	}

	Vec2 DrawBoard::getSize() const {
		return imGuiSize_;
	}

	void DrawBoard::draw(Graphic& graphic) {
		const float squareSize = TetrisData::getInstance().getTetrisSquareSize();
		const float borderSize = TetrisData::getInstance().getTetrisBorderSize();
			
		const int columns = tetrisBoard_.getColumns();
		const int rows = tetrisBoard_.getRows();

		const float infoSize = squareSize * 5.f;
		const float boardWidth = squareSize * columns;
			
		graphic.backMatrix() = glm::translate(graphic.backMatrix(), {pos_, 0});
		graphic.backMatrix() = glm::scale(graphic.backMatrix(), {imGuiSize_.x / width_, imGuiSize_.x / width_, 1.f});

		//restart(player); // Must be called after variables are defined.

		// Draw the player area.
		float x = borderSize;
		float y = borderSize;
		addRectangle(graphic,
			x, y,
			boardWidth + infoSize + middleDistance + rightDistance, squareSize * (rows - 2),
			TetrisData::getInstance().getPlayerAreaColor());

		// Draw the outer square.
		x = borderSize;
		y = borderSize;
		addRectangle(graphic,
			x, y,
			squareSize * columns, squareSize * (rows - 2),
			TetrisData::getInstance().getOuterSquareColor());

		// Draw the inner squares.
		for (int row = 0; row < rows - 2; ++row) {
			for (int column = 0; column < columns; ++column) {
				x = borderSize + squareSize * column + squareSize * 0.1f;
				y = borderSize + squareSize * row + squareSize * 0.1f;
				addRectangle(graphic,
					x, y,
					squareSize * 0.8f, squareSize * 0.8f,
					TetrisData::getInstance().getInnerSquareColor());
			}
		}

		// Draw the block start area.
		x = borderSize;
		y = borderSize + squareSize * (rows - 4);
		addRectangle(graphic,
			x, y,
			squareSize * columns, squareSize * 2,
			TetrisData::getInstance().getStartAreaColor());
			

		// Draw the preview block area.
		x = borderSize + boardWidth + middleDistance;
		y = borderSize + squareSize * (rows - 4) - (squareSize * 5 + middleDistance);
		addRectangle(graphic,
			x, y,
			infoSize, infoSize,
			TetrisData::getInstance().getStartAreaColor());

		const Color borderColor = TetrisData::getInstance().getBorderColor();

		// Add border.
		// Left-up corner.
		x = 0.f;
		y = height_ - borderSize;
		addSquare(graphic,
			x, y,
			borderSize,
			borderColor);

		// Right-up corner.
		x = width_ - borderSize;
		y = height_ - borderSize;
		addSquare(graphic,
			x, y,
			borderSize,
			borderColor);

		// Left-down corner.
		x = 0.f;
		y = 0.f;
		addSquare(graphic,
			x, y,
			borderSize,
			borderColor);

		// Right-down corner.
		x = width_ - borderSize;
		y = 0.f;
		addSquare(graphic,
			x, y,
			borderSize,
			borderColor);

		// Up.
		x = borderSize;
		y = height_ - borderSize;
		addRectangle(graphic,
			x, y,
			width_ - 2 * borderSize, borderSize,
			borderColor);

		// Down.
		x = borderSize;
		y = 0.f;
		addRectangle(graphic,
			x, y,
			width_ - 2 * borderSize, borderSize,
			borderColor);

		// Left.
		x = 0.f;
		y = borderSize;
		addRectangle(graphic,
			x, y,
			borderSize, height_ - 2 * borderSize,
			borderColor);

		// Right.
		x = width_ - borderSize;
		y = borderSize;
		addRectangle(graphic,
			x, y,
			borderSize, height_ - 2 * borderSize,
			borderColor);

		for (int i = 0; i < tetrisBoard_.getRows(); ++i) {
			for (int j = 0; j < tetrisBoard_.getColumns(); ++j) {
				x = borderSize + squareSize * j;
				y = borderSize + squareSize * i;
				if (tetrisBoard_.getBlockType(i, j) != BlockType::EMPTY
					&& tetrisBoard_.getBlockType(i, j) != BlockType::WALL) {
					
					addSquare(graphic,
						x, y,
						squareSize,
						sdl::WHITE);
				}
			}
		}
	}

}
