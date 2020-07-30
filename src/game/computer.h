#ifndef MWETRIS_GAME_COMPUTER_H
#define MWETRIS_GAME_COMPUTER_H

#include "device.h"
#include "block.h"
#include "tetrisboard.h"
#include "ai.h"

#include <calc/calculator.h>

#include <vector>
#include <string>
#include <future>

namespace mwetris::game {

	class Computer : public Device {
	public:
		Computer() = default;

		Computer(const tetris::Ai& ai);

		Input getInput() const override;

		std::string getName() const override;

		void update(const tetris::TetrisBoard& board) override;

		bool isAi() const override {
			return true;
		}

	private:
		static tetris::Ai::State calculateBestState(tetris::TetrisBoard board, tetris::Ai ai, int depth);

		// Calculate and return the best input to achieve the current state.
		Input calculateInput(tetris::Ai::State state) const;

		int currentTurn_{};
		Input input_{};
		tetris::Ai::State latestState_{};
		tetris::Block latestBlock_;
		tetris::Ai ai_;
		bool activeThread_{};
		std::future<tetris::Ai::State> handle_;
	};

}

#endif
