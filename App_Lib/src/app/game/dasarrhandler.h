#ifndef APP_GAME_DASARRHANDLER_H
#define APP_GAME_DASARRHANDLER_H

#include "input.h"

#include <memory>

namespace app::game {

	class DasArrHandler {
	public:
		DasArrHandler(double das, double arr);

		void update(double timeStep, const KeyState& keyState);
		bool doAction();
		void reset();

	private:
		double das_;                 // Delayed Auto Shift (initial delay)
		double arr_;                 // Auto Repeat Rate (repeat interval)
		double time_ = 0.0;          // Total elapsed time
		double lastActionTime_ = 0.0;

		bool inDelayPhase_ = true;
		bool previouslyHeld_ = false;
		bool actionReady_ = false;
	};

	using DasArrHandlerPrt = std::shared_ptr<DasArrHandler>;

}

#endif
