#ifndef AI_H
#define AI_H

#include "rawtetrisboard.h"

#include <calc/calculator.h>

#include <string>
#include <vector>

namespace tetris {

	struct RowRoughness {
		int holes;
		int rowSum;
	};

	struct ColumnRoughness {
		int holes_;
		int bumpiness;
	};

	RowRoughness calculateRowRoughness(const RawTetrisBoard& board, int highestUsedRow);
	ColumnRoughness calculateColumnHoles(const RawTetrisBoard& board, int highestUsedRow);
	int calculateHighestUsedRow(const RawTetrisBoard& board);
	float calculateBlockMeanHeight(const Block& block);
	int calculateBlockEdges(const RawTetrisBoard& board, const Block& block);

	int calculateLandingHeight(const Block& block);
	int calculateErodedPieces(const RawTetrisBoard& board);
	int calculateRowTransitions(const RawTetrisBoard& board);
	int calculateColumnTransitions(const RawTetrisBoard& board);
	int calculateNumberOfHoles(const RawTetrisBoard& board);
	int calculateCumulativeWells(const RawTetrisBoard& board);
	int calculateHoleDepth(const RawTetrisBoard& board);
	int calculateRowHoles(const RawTetrisBoard& board);

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

		std::string getName() const {
			return name_;
		}

		std::string getValueFunction() const {
			return valueFunction_;
		}

		const calc::Calculator& getCalculator() const {
			return calculator_;
		}

		struct State {
			State() = default;

			State(int left, int rotations);

			int left{};
			int rotationLeft{};
			float value_{std::numeric_limits<float>::lowest()};
		};

		State calculateBestState(const RawTetrisBoard& board, int depth);
		float moveBlockToGroundCalculateValue(const State& state, RawTetrisBoard& board);

	private:
		void initCalculator(bool allowException);
		void initAiParameters(const calc::Calculator& calculator, const calc::Cache& cache);

		State calculateBestStateRecursive(const RawTetrisBoard& board, int depth);

		std::string name_;
		std::string valueFunction_;

		calc::Calculator calculator_;
		calc::Cache cache_;
		AiParameters parameters_{};
	};

	void moveBlockToBeforeImpact(const Ai::State& state, RawTetrisBoard& board);

}

#endif
