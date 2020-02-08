#ifndef GRAPHICBOARD_H
#define GRAPHICBOARD_H

#include "boardshader.h"
#include "../game/player.h"
#include "drawrow.h"
#include "tetrisboard.h"
#include "boardshader.h"
#include "drawblock.h"
#include "drawtext.h"
#include "boardbatch.h"
#include "../types.h"

#include <sdl/font.h>
#include <sdl/text.h>
#include <sdl/sprite.h>
#include <mw/signal.h>

#include <random>
#include <string>
#include <list>

namespace tetris {

	class GameGraphic {
	public:
		GameGraphic() = default;

		~GameGraphic();

		void restart(BoardBatch& boardBatch, Player& player, float x, float y, bool showPoints);

		void restart(Player& player);

		void update(int clearedRows, int points, int level);

		float getWidth() const {
			return width_;
		}

		float getHeight() const {
			return height_;
		}

		void update(float deltaTime, BoardBatch& dynamicBoardBatch);

		void setMiddleMessage(const sdl::Text& text);

		void showPoints() {
			showPoints_ = true;
		}

		void hidePoints() {
			showPoints_ = false;
		}

		void setName(std::string name);

		void callback(BoardEvent gameEvent, const TetrisBoard& tetrisBoard);

		void drawText(BoardBatch& batch);

		void drawMiddleText(BoardBatch& batch);

	private:
		void initStaticBackground(BoardBatch& boardBatch, float lowX, float lowY, Player& player);

		void addDrawRowAtTheTop(const TetrisBoard& tetrisBoard, int nbr);

		void addEmptyRowTop(const TetrisBoard& tetrisBoard);

		void addDrawRowBottom(const TetrisBoard& tetrisBoard, int row);

		std::list<DrawRowPtr> rows_;
		std::list<DrawRowPtr> freeRows_;

		DrawText textLevel_, textPoints_, textClearedRows_, name_, middleMessage_;
		DrawText middleText_;
		DrawBlock currentBlock_, nextBlock_, downBlock_;
		int level_, points_, clearedRows_;
		Block latestBlockDownGround_;
		bool blockDownGround_;
		bool showDownBlock_;

		mw::signals::Connection connection_;
		float width_, height_;
		float lowX_, lowY_;
		bool showPoints_;
	};

}

#endif // GRAPHICBOARD_H
