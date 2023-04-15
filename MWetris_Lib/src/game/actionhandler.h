#ifndef MWETRIS_GAME_ACTIONHANDLER_H
#define MWETRIS_GAME_ACTIONHANDLER_H

namespace mwetris::game {

	class ActionHandler {
	public:
		ActionHandler() = default;
		ActionHandler(double waitingTime, bool rebound);

		void update(double timeStep, bool action);
		bool doAction();
		double getWaitingTime() const;
		void setWaitingTime(double waitingTime);

		void reset();

	private:
		bool action_ = false;
		double lastTime_ = 0.0;
		double waitingTime_ = 0.0;
		double time_ = 0.0;
		bool rebound_ = false;
		bool lastAction_ = false;
	};

}

#endif
