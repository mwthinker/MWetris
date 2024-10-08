#ifndef APP_GAME_COMPUTER_H
#define APP_GAME_COMPUTER_H

#include "input.h"

#include <tetris/ai.h>
#include <tetris/block.h>
#include <tetris/tetrisboard.h>

#include <memory>

namespace app::game {

	class Computer;
	using ComputerPtr = std::shared_ptr<Computer>;

	class Computer {
	public:
		explicit Computer(const tetris::Ai& ai);

		Input getInput() const;

		void onGameboardEvent(const tetris::TetrisBoard& board, tetris::BoardEvent, int value);

	private:
		bool isHorizontalMoveDone(const tetris::TetrisBoard& board) const;
		bool isRotationDone(const tetris::TetrisBoard& board) const;

		Input input_{};
		tetris::Ai::State state_{};
		tetris::Block block_;
		tetris::Ai ai_;
	};

}

#endif
