#ifndef MWETRIS_GAME_ACTIONHANDLER_H
#define MWETRIS_GAME_ACTIONHANDLER_H

namespace tetris::game {

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
		bool action_{};
		double lastTime_{};
		double waitingTime_{};
		double time_{};
		bool rebound_{};
		bool lastAction_{};
	};

}

#endif
