#include "gamegraphic.h"
#include "tetrisboard.h"
#include "../game/player.h"
#include "../tetrisdata.h"

#include <limits>
#include <string>

#include <sdl/graphic.h>

namespace tetris::graphic {

	namespace {

		int calculateWitdh(const Block& block) {
			int min = std::numeric_limits<int>::max();
			int max = 0;
			for (const Square& sq : block) {
				if (sq.column > max) {
					max = sq.column;
				}
				if (sq.column < min) {
					min = sq.column;
				}
			}
			return max + 1 - min;
		}

		int calculateHighest(const Block& block) {
			int max = 0;
			for (const Square& sq : block) {
				if (sq.row > max) {
					max = sq.row;
				}
			}
			return max;
		}

		int calculateLeftColumn(const Block& block) {
			int min = std::numeric_limits<int>::max();
			for (const Square& sq : block) {
				if (sq.column < min) {
					min = sq.column;
				}
			}
			return min;
		}

	}

	GameGraphic::~GameGraphic() {
		connection_.disconnect();
	}

	/*
	void GameGraphic::restart(BoardBatch& boardBatch, Player& player, float x, float y, bool showPoints) {
		connection_.disconnect();
		connection_ = player.addGameEventListener(std::bind(&GameGraphic::callback, this, std::placeholders::_1, std::placeholders::_2));

		initStaticBackground(boardBatch, x, y, player);
	}
	*/

	void GameGraphic::restart(game::Player& player) {
		const float squareSize = TetrisData::getInstance().getTetrisSquareSize();
		const int rows = player.getTetrisBoard().getRows();
		const float borderSize = TetrisData::getInstance().getTetrisBorderSize();

		rows_.clear();
		freeRows_.clear();

		// Add rows to represent the board.
		// Add free rows to represent potential rows, e.g. the board receives external rows.
		/*
		for (int row = 0; row < rows; ++row) {
			auto drawRow = std::make_shared<DrawRow>(row, player.getTetrisBoard(), squareSize, lowX_ + borderSize, lowY_ + borderSize);
			auto freeRow = std::make_shared<DrawRow>(*drawRow);
			freeRow->clear(); // Make all elements to only contain blocktype empty squares.
			rows_.push_back(drawRow);
			freeRows_.push_back(freeRow);
		}
		
		
		currentBlock_ = DrawBlock(player.getTetrisBoard().getBlock(), player.getTetrisBoard().getRows(), squareSize,
			lowX_ + borderSize, lowY_ + borderSize, false);
		currentBlock_.update(player.getTetrisBoard().getBlock());

		nextBlock_.update(Block(player.getTetrisBoard().getNextBlockType(), 0, 0));	
	
		RawTetrisBoard board = player.getTetrisBoard();
		board.update(Move::DOWN_GROUND);
		downBlock_ = DrawBlock(board.getBlock(), board.getRows(), squareSize,
			lowX_ + borderSize, lowY_ + borderSize, false);
		downBlock_.setColor(TetrisData::getInstance().getDownBlockColor());
		downBlock_.update(board.getBlock());
		showDownBlock_ = TetrisData::getInstance().isShowDownBlock();
		*/
	}

	/*
	void GameGraphic::initStaticBackground(BoardBatch& staticBoardBatch, float lowX, float lowY, Player& player) {
		const float squareSize = TetrisData::getInstance().getTetrisSquareSize();
		const float borderSize = TetrisData::getInstance().getTetrisBorderSize();

		const TetrisBoard& tetrisBoard = player.getTetrisBoard();
		const int columns = tetrisBoard.getColumns();
		const int rows = tetrisBoard.getRows();

		const float middleDistance = 5;
		const float rightDistance = 5;
		const float infoSize = squareSize * 5;
		const float boardWidth = squareSize * columns;

		lowX_ = lowX;
		lowY_ = lowY;

		width_ = squareSize * columns + infoSize + borderSize * 2 + middleDistance + rightDistance;
		height_ = squareSize * (rows - 2) + borderSize * 2;

		restart(player); // Must be called after variables are defined.

		// Draw the player area.
		float x = lowX + borderSize;
		float y = lowY * 0.5f + borderSize;

		staticBoardBatch.addRectangle(
			x, y,
			boardWidth + infoSize + middleDistance + rightDistance, squareSize * (rows - 2),
			TetrisData::getInstance().getPlayerAreaColor());

		// Draw the outer square.
		x = lowX + borderSize;
		y = lowY + borderSize;
		staticBoardBatch.addRectangle(
			x, y,
			squareSize * columns, squareSize * (rows - 2),
			TetrisData::getInstance().getOuterSquareColor());

		// Draw the inner squares.
		for (int row = 0; row < rows - 2; ++row) {
			for (int column = 0; column < columns; ++column) {
				x = lowX + borderSize + squareSize * column + squareSize * 0.1f;
				y = lowY + borderSize + squareSize * row + squareSize * 0.1f;
				staticBoardBatch.addRectangle(
					x, y,
					squareSize * 0.8f, squareSize * 0.8f,
					TetrisData::getInstance().getInnerSquareColor());
			}
		}

		// Draw the block start area.
		x = lowX + borderSize;
		y = lowY + borderSize + squareSize * (rows - 4);
		staticBoardBatch.addRectangle(
			x, y,
			squareSize * columns, squareSize * 2,
			TetrisData::getInstance().getStartAreaColor());

		// Draw the preview block area.
		x = lowX + borderSize + boardWidth + middleDistance;
		y = lowY + borderSize + squareSize * (rows - 4) - (squareSize * 5 + middleDistance);
		staticBoardBatch.addRectangle(
			x, y,
			infoSize, infoSize,
			TetrisData::getInstance().getStartAreaColor());

		nextBlock_ = DrawBlock(Block(tetrisBoard.getNextBlockType(), 0, 0),
			tetrisBoard.getRows(),
			squareSize, x + squareSize * 2.5f, y + squareSize * 2.5f,
			true);
		nextBlock_.update(Block(player.getTetrisBoard().getNextBlockType(), 0, 0));
		
		const Color borderColor = TetrisData::getInstance().getBorderColor();

		// Add border.
		// Left-up corner.
		x = lowX;
		y = lowY + height_ - borderSize;
		staticBoardBatch.addSquare(
			x, y,
			borderSize,
			borderColor);

		// Right-up corner.
		x = lowX + width_ - borderSize;
		y = lowY + height_ - borderSize;
		staticBoardBatch.addSquare(
			x, y,
			borderSize,
			borderColor);

		// Left-down corner.
		x = lowX;
		y = lowY;
		staticBoardBatch.addSquare(
			x, y,
			borderSize,
			borderColor);

		// Right-down corner.
		x = lowX + width_ - borderSize;
		y = lowY;
		staticBoardBatch.addSquare(
			x, y,
			borderSize,
			borderColor);

		// Up.
		x = lowX + borderSize;
		y = lowY + height_ - borderSize;
		staticBoardBatch.addRectangle(
			x, y,
			width_ - 2 * borderSize, borderSize,
			borderColor);

		// Down.
		x = lowX + borderSize;
		y = lowY;
		staticBoardBatch.addRectangle(
			x, y,
			width_ - 2 * borderSize, borderSize,
			borderColor);

		// Left.
		x = lowX;
		y = lowY + borderSize;
		staticBoardBatch.addRectangle(
			x, y,
			borderSize, height_ - 2 * borderSize,
			borderColor);

		// Right.
		x = lowX + width_ - borderSize;
		y = lowY + borderSize;
		staticBoardBatch.addRectangle(
			x, y,
			borderSize, height_ - 2 * borderSize,
			borderColor);
	}

	
	void GameGraphic::callback(BoardEvent gameEvent, const TetrisBoard& tetrisBoard) {
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
				size_t highestRow = tetrisBoard.getBoardVector().size() / tetrisBoard.getColumns();
				assert(rows_.size() - highestRow >= 0); // Something is wrong. Should not be posssible.
				for (int i = 0; i < (int) rows_.size() - highestRow; ++i) { // Remove unneeded empty rows at the top.
					rows_.pop_back();
				}
			}
			break;
			case BoardEvent::NEXT_BLOCK_UPDATED:
				nextBlock_.update(Block(tetrisBoard.getNextBlockType(), 0, 0));
				break;
			case BoardEvent::CURRENT_BLOCK_UPDATED: [[fallthrough]];
			case BoardEvent::PLAYER_MOVES_BLOCK_ROTATE: [[fallthrough]];
			case BoardEvent::PLAYER_MOVES_BLOCK_LEFT: [[fallthrough]];
			case BoardEvent::PLAYER_MOVES_BLOCK_RIGHT:
				currentBlock_.update(tetrisBoard.getBlock());
				{
					RawTetrisBoard board = tetrisBoard;
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
				[[fallthrough]];
			case BoardEvent::GRAVITY_MOVES_BLOCK:
				currentBlock_.update(tetrisBoard.getBlock());
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
	}
	*/
	void GameGraphic::addDrawRowAtTheTop(const TetrisBoard& tetrisBoard, int nbr) {
		for (int i = 0; i < nbr; ++i) {
			addEmptyRowTop(tetrisBoard); // Add them in ascending order.
		}
	}

	/*
	void GameGraphic::update(float deltaTime, BoardBatch& dynamicBoardBatch) {
		currentBlock_.update(deltaTime);

		if (showDownBlock_) {
			dynamicBoardBatch.add(downBlock_.getVertexes());
		}

		dynamicBoardBatch.add(currentBlock_.getVertexes());
		dynamicBoardBatch.add(nextBlock_.getVertexes());	

		// Update the animation for the rows still showing animations.
		for (auto& rowPtr : freeRows_) {
			if (rowPtr->isActive()) {
				rowPtr->update(deltaTime);
				dynamicBoardBatch.add(rowPtr->getVertexes());
			}
		}

		// Update the rows for representing the tetris board.
		for (auto& rowPtr : rows_) {
			rowPtr->update(deltaTime);
			dynamicBoardBatch.add(rowPtr->getVertexes());
		}
	}
	*/

	void GameGraphic::addEmptyRowTop(const TetrisBoard& tetrisBoard) {
		assert(!freeRows_.empty()); // Should never be empty.
		if (!freeRows_.empty()) { // Just in case empty, but the game should be over anyway.
			auto drawRow = freeRows_.back();
			freeRows_.pop_back();
			drawRow->init(static_cast<int>(rows_.size()), tetrisBoard);
			rows_.push_back(drawRow);
		}
	}

	void GameGraphic::addDrawRowBottom(const TetrisBoard& tetrisBoard, int row) {
		assert(!freeRows_.empty()); // Should never be empty.
		if (!freeRows_.empty()) {
			auto drawRow = freeRows_.back();
			freeRows_.pop_back();
			drawRow->init(row, tetrisBoard);
			rows_.push_front(drawRow); // Add as the lowest row, i.e. on the bottom.
		}
	}

} // Namespace tetris.
