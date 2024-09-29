#include "timerhandler.h"

#include <functional>
#include <vector>
#include <algorithm>

#include <spdlog/spdlog.h>

namespace app {

	int TimeHandler::id_ = 0;

	void TimeHandler::update(double duration) {
		currentTime_ += duration;

		std::erase_if(scheduledCallbacks_, [this](TimeEvent& timeEvent) {
			if (timeEvent.eventTime + timeEvent.interval <= currentTime_) {
				spdlog::debug("TimeEvent id {}, {}s >= {}s", timeEvent.id, timeEvent.eventTime + timeEvent.interval, currentTime_);

				if (timeEvent.maxNbr > 0) {
					--timeEvent.maxNbr;
					timeEvent.eventTime = currentTime_;
				}
				timeEvent.callback();
				return timeEvent.maxNbr <= 0;
			}
			return false;
		});
	}

	TimeHandler::Key TimeHandler::schedule(Callback callback, double delay) {
		return scheduleRepeat(callback, delay, 1);
	}

	TimeHandler::Key TimeHandler::scheduleRepeat(Callback callback, double interval, int maxNbr) {
		auto& timeEvent = scheduledCallbacks_.emplace_back(TimeEvent{
			.id = ++id_,
			.eventTime = currentTime_,
			.interval = interval,
			.callback = callback,
			.maxNbr = maxNbr
		});

		return Key{timeEvent.id};
	}

	bool TimeHandler::removeCallback(Key key) {
		auto size = std::erase_if(scheduledCallbacks_, [id = key.id_](const TimeEvent& timeEvent) {
			return timeEvent.id == id;
		});
		return size > 0;
	}

	double TimeHandler::getCurrentTime() const {
		return currentTime_;
	}

	void TimeHandler::reset() {
		currentTime_ = 0.0;
		scheduledCallbacks_.clear();
	}

	bool TimeHandler::hasKey(Key key) const {
		return std::any_of(scheduledCallbacks_.begin(), scheduledCallbacks_.end(), [key](const TimeEvent& timeEvent) {
			return timeEvent.id == key.id_;
		});
	}

}
