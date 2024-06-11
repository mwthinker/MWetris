#include <benchmark/benchmark.h>

#include <tetris/tetrisboard.h>
#include <tetris/block.h>
#include <tetris/ai.h>

#include <vector>

using namespace tetris;

namespace {

	constexpr int TetrisWidth = 10;
	constexpr int TetrisHeight = 24;

	constexpr BlockType charToBlockType(char key) {
		switch (key) {
			case 'z': case 'Z':
				return BlockType::Z;
			case 'w': case 'W':
				return BlockType::Wall;
			case 't': case 'T':
				return BlockType::T;
			case 's': case 'S':
				return BlockType::S;
			case 'o': case 'O':
				return BlockType::O;
			case 'l': case 'L':
				return BlockType::L;
			case 'j': case 'J':
				return BlockType::J;
			case 'I': case 'i':
				return BlockType::I;
			default:
				return BlockType::Empty;
		}
	}

	TetrisBoard createTetrisBoard() {
		std::vector<BlockType> blockTypes;
		for (char chr : std::string_view{"TTOOZZSSEIETOOSZSEEEEEESSEEEEEEEES"}) {
			blockTypes.push_back(charToBlockType(chr));
		}
		return TetrisBoard{blockTypes,
			TetrisWidth, TetrisHeight,
			Block{BlockType::J, 4, 18, 0},
			BlockType::L
		};
	}

}

class MyFixture : public benchmark::Fixture {
public:
	void SetUp(const ::benchmark::State& state) override {

	}

	void TearDown(const ::benchmark::State& state) override {

	}

};

BENCHMARK_F(MyFixture, copyBoard)(benchmark::State& state) {
	TetrisBoard board{TetrisWidth, TetrisHeight, BlockType::S, BlockType::J};
	for (auto _ : state) {
		TetrisBoard copyBoard = board;
		board.update(Move::Down);
	}
}

BENCHMARK_F(MyFixture, copyBlock)(benchmark::State& state) {
	sizeof(Block);
	sizeof(int);
	sizeof(TetrisBoard);
	sizeof(int_fast32_t);

	for (auto _ : state) {
		BlockType blockTypes[] = { BlockType::I, BlockType::J, BlockType::L, BlockType::O, BlockType::S, BlockType::T, BlockType::Z};

		for (int i = 0; i < 10; ++i) {
			Block block{blockTypes[i % sizeof(blockTypes)], 0, 0};
			for (int j = 0; j < 4; ++j) {
				block.rotateLeft();
			}
			for (int j = 0; j < 4; ++j) {
				block.rotateRight();
			}
			for (int j = 0; j < 8; ++j) {
				block.moveDown();
			}
		}
	}
}

BENCHMARK_F(MyFixture, oldAiFunction)(benchmark::State& state) {
	auto board = createTetrisBoard();
	int highestUsedRow = calculateHighestUsedRow(board);

	for (auto _ : state) {
		calculateColumnHoles(board, highestUsedRow);
		calculateRowRoughness(board, highestUsedRow);
		calculateBlockMeanHeight(board.getBlock());
		calculateHighestUsedRow(board);
		calculateBlockEdges(board, board.getBlock());
	}
}

BENCHMARK_F(MyFixture, calculateColumnHoles)(benchmark::State& state) {
	auto board = createTetrisBoard();
	int highestUsedRow = calculateHighestUsedRow(board);

	for (auto _ : state) {
		calculateColumnHoles(board, highestUsedRow);
	}
}

BENCHMARK_F(MyFixture, calculateRowRoughness)(benchmark::State& state) {
	auto board = createTetrisBoard();
	int highestUsedRow = calculateHighestUsedRow(board);

	for (auto _ : state) {
		calculateRowRoughness(board, highestUsedRow);
	}
}

BENCHMARK_F(MyFixture, calculateBlockMeanHeight)(benchmark::State& state) {
	auto board = createTetrisBoard();

	for (auto _ : state) {
		calculateBlockMeanHeight(board.getBlock());
	}
}

BENCHMARK_F(MyFixture, calculateHighestUsedRow)(benchmark::State& state) {
	auto board = createTetrisBoard();

	for (auto _ : state) {
		calculateHighestUsedRow(board);
	}
}

BENCHMARK_F(MyFixture, calculateBlockEdges)(benchmark::State& state) {
	auto board = createTetrisBoard();

	for (auto _ : state) {
		calculateBlockEdges(board, board.getBlock());
	}
}

BENCHMARK_F(MyFixture, newAiFunctions)(benchmark::State& state) {
	auto board = createTetrisBoard();
	int highestUsedRow = calculateHighestUsedRow(board);
	board.update(Move::DownGround);

	for (auto _ : state) {
		calculateLandingHeight(board.getBlock());
		calculateErodedPieces(board);
		board.update(Move::DownGround);
		calculateRowTransitions(board);
		calculateColumnTransitions(board);
		calculateNumberOfHoles(board);
		calculateCumulativeWells(board);
		calculateHoleDepth(board);
		calculateRowHoles(board);
	}
}

BENCHMARK_F(MyFixture, calculateLandingHeight)(benchmark::State& state) {
	auto board = createTetrisBoard();

	for (auto _ : state) {
		calculateLandingHeight(board.getBlock());
	}
}

BENCHMARK_F(MyFixture, calculateErodedPieces)(benchmark::State& state) {
	auto board = createTetrisBoard();

	for (auto _ : state) {
		calculateErodedPieces(board);
	}
}

BENCHMARK_F(MyFixture, calculateRowTransitions)(benchmark::State& state) {
	auto board = createTetrisBoard();

	for (auto _ : state) {
		calculateRowTransitions(board);
	}
}

BENCHMARK_F(MyFixture, calculateColumnTransitions)(benchmark::State& state) {
	auto board = createTetrisBoard();

	for (auto _ : state) {
		calculateColumnTransitions(board);
	}
}

BENCHMARK_F(MyFixture, calculateNumberOfHoles)(benchmark::State& state) {
	auto board = createTetrisBoard();

	for (auto _ : state) {
		calculateNumberOfHoles(board);
	}
}

BENCHMARK_F(MyFixture, calculateCumulativeWells)(benchmark::State& state) {
	auto board = createTetrisBoard();

	for (auto _ : state) {
		calculateCumulativeWells(board);
	}
}

BENCHMARK_F(MyFixture, calculateHoleDepth)(benchmark::State& state) {
	auto board = createTetrisBoard();
	board.update(Move::DownGround);

	for (auto _ : state) {
		calculateHoleDepth(board);
	}
}

BENCHMARK_F(MyFixture, calculateBestState)(benchmark::State& state) {
	auto board = createTetrisBoard();
	board.update(Move::DownGround);

	Ai ai;
	for (auto _ : state) {
		ai.calculateBestState(board, 1);
	}
}
