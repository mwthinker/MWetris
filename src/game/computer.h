#ifndef MWETRIS_GAME_COMPUTER_H
#define MWETRIS_GAME_COMPUTER_H

#include "device.h"
#include "block.h"
#include "tetrisboard.h"
#include "ai.h"
#include "tetrisboardwrapper.h"

#include <calc/calculator.h>

#include <vector>
#include <string>
#include <future>

namespace tetris::game {

	class Computer : public Device {
	public:
		Computer() = default;

		Computer(const Ai& ai);

		Input getInput() const override;

		std::string getName() const override;

		void update(const TetrisBoardWrapper& board) override;

		bool isAi() const override {
			return true;
		}

	private:
		static Ai::State calculateBestState(TetrisBoard board, Ai ai, int depth);

		// Calculate and return the best input to achieve the current state.
		Input calculateInput(Ai::State state) const;

		int currentTurn_{};
		Input input_{};
		Ai::State latestState_{};
		Block latestBlock_;
		Ai ai_;
		bool activeThread_{};
		std::future<Ai::State> handle_;
	};

}

#endif
