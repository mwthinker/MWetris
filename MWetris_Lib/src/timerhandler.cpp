#include "timerhandler.h"

#include <functional>
#include <vector>
#include <algorithm>

#include <spdlog/spdlog.h>

namespace mwetris {

	int TimeHandler::id_ = 0;

	// Update the internal time and trigger callbacks
	void TimeHandler::update(double duration) {
		currentTime_ += duration;

		std::erase_if(scheduledCallbacks_, [this](TimeEvent& timeEvent) {
			if (timeEvent.eventTime + timeEvent.interval <= currentTime_) {
				spdlog::debug("TimeEvent {}, {} >= {}", timeEvent.id, timeEvent.eventTime + timeEvent.interval, currentTime_);

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

	// Schedule a callback function to be triggered after a given duration
	TimeHandler::Key TimeHandler::schedule(Callback callback, double delay) {
		return scheduleRepeat(callback, delay, 1);
	}

	// Schedule a callback function to be repeatedly triggered at a specified interval
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

	/// @brief Remove callback associated with the key and return true if callback is found else false.
	/// @param key to the callback
	/// @return true if callback is found else false
	bool TimeHandler::removeCallback(Key key) {
		auto size = std::erase_if(scheduledCallbacks_, [id = key.id_](const TimeEvent& timeEvent) {
			return timeEvent.id == id;
		});
		return size > 0;
	}

	double TimeHandler::getCurrentTime() const {
		return currentTime_;
	}

	void TimeHandler::clear() {
		scheduledCallbacks_.clear();
	}

}
