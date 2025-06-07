#include "dasarrhandler.h"

namespace app::game {

	DasArrHandler::DasArrHandler(double das, double arr)
		: das_{das}
		, arr_{arr} {
	}

	void DasArrHandler::update(double timeStep, const KeyState& keyState) {
		time_ += timeStep;

		if (keyState.pressed && !previouslyHeld_) {
			actionReady_ = true;
			lastActionTime_ = time_;
			inDelayPhase_ = true;
		} else if (keyState.held) {
			double elapsed = time_ - lastActionTime_;
			if (inDelayPhase_) {
				if (elapsed >= das_) {
					actionReady_ = true;
					lastActionTime_ = time_;
					inDelayPhase_ = false;
				}
			} else {
				if (elapsed >= arr_) {
					actionReady_ = true;
					lastActionTime_ = time_;
				}
			}
		}

		if (keyState.released) {
			inDelayPhase_ = true;
			previouslyHeld_ = false;
			return;
		}

		previouslyHeld_ = keyState.held;
	}

	void DasArrHandler::reset() {
		time_ = 0.0;
		inDelayPhase_ = true;
		actionReady_ = false;
		previouslyHeld_ = false;
		lastActionTime_ = 0.0;
	}

	bool DasArrHandler::doAction() {
		if (actionReady_) {
			actionReady_ = false;
			return true;
		}
		return false;
	}

}
