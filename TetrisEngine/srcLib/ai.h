#ifndef AI_H
#define AI_H

#include "tetrisboard.h"

#include <calc/calculator.h>

#include <string>
#include <vector>
#include <limits>

namespace tetris {

	struct RowRoughness {
		int holes;
		int rowSum;
	};

	struct ColumnRoughness {
		int holes_;
		int bumpiness;
	};

	RowRoughness calculateRowRoughness(const TetrisBoard& board, int highestUsedRow);
	ColumnRoughness calculateColumnHoles(const TetrisBoard& board, int highestUsedRow);
	int calculateHighestUsedRow(const TetrisBoard& board);
	float calculateBlockMeanHeight(const Block& block);
	int calculateBlockEdges(const TetrisBoard& board, const Block& block);

	int calculateLandingHeight(const Block& block);
	int calculateErodedPieces(const TetrisBoard& board);
	int calculateRowTransitions(const TetrisBoard& board);
	int calculateColumnTransitions(const TetrisBoard& board);
	int calculateNumberOfHoles(const TetrisBoard& board);
	int calculateCumulativeWells(const TetrisBoard& board);
	int calculateHoleDepth(const TetrisBoard& board);
	int calculateRowHoles(const TetrisBoard& board);

	struct AiParameters {
		bool landingHeight;
		bool erodedPieces;
		bool rowHoles;
		bool columnHoles;
		bool holes;
		bool cumulativeWells;
		bool holeDepth;
	};

	class Ai {
	public:
		Ai();

		Ai(std::string name, std::string valueFunction, bool allowException = false);

		const std::string& getName() const {
			return name_;
		}

		const std::string& getValueFunction() const {
			return valueFunction_;
		}

		const calc::Calculator& getCalculator() const {
			return calculator_;
		}

		struct State {
			int left = 0;
			int rotationLeft = 0;
			float value = std::numeric_limits<float>::lowest();
		};

		State calculateBestState(const TetrisBoard& board, int depth);
		float moveBlockToGroundCalculateValue(const State& state, TetrisBoard& board);

	private:
		void initCalculator(bool allowException);
		void initAiParameters(const calc::Calculator& calculator, const calc::Cache& cache);

		State calculateBestStateRecursive(const TetrisBoard& board, int depth);

		std::string name_;
		std::string valueFunction_;

		calc::Calculator calculator_;
		calc::Cache cache_;
		AiParameters parameters_{};
	};

	template <typename Board>
	void moveBlockToBeforeImpact(const Ai::State& state, Board& board) {
		for (int i = 0; i < state.rotationLeft; ++i) {
			board.update(Move::RotateLeft);
		}
		for (int i = 0; i < state.left; ++i) {
			board.update(Move::Left);
		}
		for (int i = 0; i < -1 * state.left; ++i) {
			board.update(Move::Right);
		}
		board.update(Move::DownGround);
	}

}

#endif
