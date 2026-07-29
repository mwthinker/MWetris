#include "dasarrhandler.h"

#include <cmath>
#include <stdexcept>

namespace app::game {

	DasArrHandler::DasArrHandler(double das, double arr) {
		setTiming(das, arr);
	}

	double DasArrHandler::getDas() const {
		return das_;
	}

	double DasArrHandler::getArr() const {
		return arr_;
	}

	void DasArrHandler::setTiming(double das, double arr) {
		if (!std::isfinite(das) || das <= 0.0 || !std::isfinite(arr) || arr <= 0.0) {
			throw std::invalid_argument{"DAS and ARR must be finite positive values"};
		}
		if (das_ == das && arr_ == arr) {
			return;
		}

		das_ = das;
		arr_ = arr;
		reset();
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
