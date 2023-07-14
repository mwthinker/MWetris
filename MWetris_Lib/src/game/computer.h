#ifndef MWETRIS_GAME_COMPUTER_H
#define MWETRIS_GAME_COMPUTER_H

#include "device.h"
#include "block.h"
#include "tetrisboard.h"
#include "ai.h"

namespace mwetris::game {

	class Computer;
	using ComputerPtr = std::shared_ptr<Computer>;

	class Computer : public Device {
	public:
		Computer() = default;

		Computer(const tetris::Ai& ai);

		Input getInput() const override;

		const char* getName() const override;

		void onGameboardEvent(const tetris::TetrisBoard& board, tetris::BoardEvent, int value) override;

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
