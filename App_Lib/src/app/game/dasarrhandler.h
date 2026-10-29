#ifndef APP_GAME_DASARRHANDLER_H
#define APP_GAME_DASARRHANDLER_H

namespace app::game {

	class DasArrHandler {
	public:
		DasArrHandler(double das, double arr);

		double getDas() const;
		double getArr() const;
		void setTiming(double das, double arr);

		void press();
		void update(double timeStep, bool held);
		bool doAction();
		void reset();

	private:
		double das_ = 0.0;           // Delayed Auto Shift (initial delay)
		double arr_ = 0.0;           // Auto Repeat Rate (repeat interval)
		double time_ = 0.0;

		bool inDelayPhase_ = true;
		bool active_ = false;
		bool actionReady_ = false;
	};

}

#endif
