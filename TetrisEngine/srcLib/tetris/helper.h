#ifndef HELPER_H
#define HELPER_H

#include "tetrisboard.h"
#include "block.h"

#include <vector>

namespace tetris {

	BlockType randomBlockType();

	std::vector<BlockType> generateRow(const TetrisBoard& board, double squaresPerLength);

	std::vector<BlockType> generateRow(int width, int holes);

}

#endif
