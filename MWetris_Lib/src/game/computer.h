#ifndef MWETRIS_GAME_COMPUTER_H
#define MWETRIS_GAME_COMPUTER_H

#include "block.h"
#include "tetrisboard.h"
#include "ai.h"
#include "input.h"

#include <memory>

namespace mwetris::game {

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
