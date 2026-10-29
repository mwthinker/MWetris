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

		const bool wasActive = active_;
		das_ = das;
		arr_ = arr;
		reset();
		active_ = wasActive;
	}

	void DasArrHandler::press() {
		time_ = 0.0;
		inDelayPhase_ = true;
		active_ = true;
		actionReady_ = false;
	}

	void DasArrHandler::update(double timeStep, bool held) {
		if (!held) {
			reset();
			return;
		}
		if (!active_) {
			return;
		}

		time_ += timeStep;
		const auto waitingTime = inDelayPhase_ ? das_ : arr_;
		if (time_ >= waitingTime) {
			actionReady_ = true;
			time_ = 0.0;
			inDelayPhase_ = false;
		}
	}

	void DasArrHandler::reset() {
		time_ = 0.0;
		inDelayPhase_ = true;
		actionReady_ = false;
		active_ = false;
	}

	bool DasArrHandler::doAction() {
		if (actionReady_) {
			actionReady_ = false;
			return true;
		}
		return false;
	}

}
