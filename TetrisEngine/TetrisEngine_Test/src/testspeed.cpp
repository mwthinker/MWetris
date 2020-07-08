// main() provided by Catch in file testmain.cpp.
#include <catch2/catch.hpp>

#include <tetrisboard.h>
#include <tetrisboardwrapper.h>
#include <square.h>
#include <vector>
#include <ai.h>

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

}

TEST_CASE("benchmarked", "[.][benchmark]") {

	BENCHMARK("Copy TetrisBoard") {
		TetrisBoard board{TetrisWidth, TetrisHeight, BlockType::S, BlockType::J};
		for (int i = 0; i < 100; ++i) {
			TetrisBoard copyBoard = board;
			board.update(Move::Down);
		}
	};

	BENCHMARK("Copy block") {
		BlockType blockTypes[] = {BlockType::I, BlockType::J, BlockType::L,
			BlockType::O, BlockType::S, BlockType::T, BlockType::Z};

		sizeof(Block);
		sizeof(int);
		sizeof(TetrisBoard);
		sizeof(TetrisBoardWrapper);
		sizeof(int_fast32_t);

		for (int i = 0; i < 10000; ++i) {
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
			for (int j = 0; j < 8; ++j) {
				block.moveUp();
			}
		}
	};

	sizeof(Block);

	std::vector<BlockType> blockTypes;
	for (char chr : std::string_view{"TTOOZZSSEIETOOSZSEEEEEESSEEEEEEEES"}) {
		blockTypes.push_back(charToBlockType(chr));
	}
	TetrisBoard board{blockTypes, TetrisWidth, TetrisHeight,
		Block{BlockType::J, 4, 18, 0}, BlockType::L};
	int highestUsedRow = calculateHighestUsedRow(board);
	
	board.update(Move::DownGround);

	BENCHMARK("Old ai functions") {
		calculateColumnHoles(board, highestUsedRow);
		calculateRowRoughness(board, highestUsedRow);
		calculateBlockMeanHeight(board.getBlock());
		calculateHighestUsedRow(board);
		calculateBlockEdges(board, board.getBlock());
	};

	BENCHMARK("calculateColumnHoles") {
		calculateColumnHoles(board, highestUsedRow);
	};
	BENCHMARK("calculateRowRoughness") {
		calculateRowRoughness(board, highestUsedRow);
	};
	BENCHMARK("calculateBlockMeanHeight") {
		calculateBlockMeanHeight(board.getBlock());
	};
	BENCHMARK("calculateHighestUsedRow") {
		calculateHighestUsedRow(board);
	};
	BENCHMARK("calculateBlockEdges") {
		calculateBlockEdges(board, board.getBlock());
	};

	BENCHMARK("New ai functions") {
		calculateLandingHeight(board.getBlock());
		calculateErodedPieces(board);
		board.update(Move::DownGround);
		calculateRowTransitions(board);
		calculateColumnTransitions(board);
		calculateNumberOfHoles(board);
		calculateCumulativeWells(board);
		calculateHoleDepth(board);
		calculateRowHoles(board);
	};
	
	BENCHMARK("calculateLandingHeight ") {
		calculateLandingHeight(board.getBlock());
	};
	BENCHMARK("calculateErodedPieces") {
		calculateErodedPieces(board);
	};
	BENCHMARK("calculateRowTransitions") {
		calculateRowTransitions(board);
	};
	BENCHMARK("calculateColumnTransitions") {
		calculateColumnTransitions(board);
	};
	BENCHMARK("calculateNumberOfHoles") {
		calculateNumberOfHoles(board);
	};
	BENCHMARK("calculateCumulativeWells") {
		calculateCumulativeWells(board);
	};
	BENCHMARK("calculateHoleDepth") {
		calculateHoleDepth(board);
	};

	BENCHMARK("AI calculateBestState") {
		Ai ai;
		ai.calculateBestState(board, 1);
	};

}
