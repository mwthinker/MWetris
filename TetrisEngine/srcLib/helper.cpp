#include "helper.h"
#include "random.h"

namespace tetris {

	BlockType randomBlockType() {
		Random random;
		constexpr std::array BlockTypes{
			BlockType::I, BlockType::J, BlockType::L,
			BlockType::O ,BlockType::S, BlockType::T, BlockType::Z};

		return BlockTypes[random.generateInt(0, static_cast<int>(BlockTypes.size()) - 1)];
	}

	std::vector<BlockType> generateRow(const TetrisBoard& board, double squaresPerLength) {
		const auto size = board.getColumns();

		Random random;
		std::vector<bool> row(size);
		for (int i = 0; i < size * squaresPerLength; ++i) {
			int index = random.generateInt(0, size - 1);
			int nbr = 0;
			while (nbr < size) {
				if (!row[(index + nbr) % size]) {
					row[(index + nbr) % size] = true;
					break;
				}
				++nbr;
			}
		}

		std::vector<BlockType> rows;
		// Fill the rows with block types.
		for (int i = 0; i < size; ++i) {
			auto blockType = BlockType::Empty;

			// Fill square?
			if (row[i]) {
				blockType = randomBlockType();
			}
			rows.push_back(blockType);
		}
		return rows;
	}

	std::vector<BlockType> generateRow(int width, int holes) {
		std::vector<BlockType> row(width);
		for (auto type : row) {
			type = randomBlockType();;
		}

		Random random;
		for (int i = 0; i < holes; ++i) {
			int index = random.generateInt(0, width - 1);
			if (row[index] == BlockType::Empty) {
				--i;
			} else {
				row[index] = BlockType::Empty;
			}
		}
		return row;
	}

}
