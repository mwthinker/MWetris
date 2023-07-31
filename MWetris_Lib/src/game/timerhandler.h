#ifndef MWETRIS_GAME_TIMER_H
#define MWETRIS_GAME_TIMER_H

#include <functional>
#include <vector>
#include <algorithm>

#include <spdlog/spdlog.h>

namespace mwetris::game {

    class TimeHandler {
    public:
        using Callback = std::function<void()>;

        TimeHandler() {
        }

        // Update the internal time and trigger callbacks
        void update(double duration) {
            currentTime_ += duration;

            std::erase_if(scheduledCallbacks_, [currentTime = currentTime_](TimeEvent& timeEvent) {
                spdlog::debug("{} <= {}", timeEvent.eventTime + timeEvent.interval, currentTime);
                if (timeEvent.eventTime + timeEvent.interval <= currentTime) {
                    if (timeEvent.maxNbr > 0) {
                        --timeEvent.maxNbr;
                        timeEvent.eventTime = currentTime;
                    }
                    timeEvent.callback();
                    return timeEvent.maxNbr <= 0;
                }
                return false;
            });
        }

        // Schedule a callback function to be triggered after a given duration
        void schedule(Callback callback, double delay) {
            scheduleRepeat(callback, delay, 1);
        }

        // Schedule a callback function to be repeatedly triggered at a specified interval
        void scheduleRepeat(Callback callback, double interval, int maxNbr) {
            scheduledCallbacks_.push_back(TimeEvent{
                .eventTime = currentTime_,
                .interval = interval,
                .callback = callback,
                .maxNbr = maxNbr
            });
        }

        // Get the current time
        double getCurrentTime() const {
            return currentTime_;
        }

        void clear() {
            scheduledCallbacks_.clear();
        }

    private:
        double currentTime_ = 0.0;

        struct TimeEvent {
            double eventTime;
            double interval;
            Callback callback;
            int maxNbr;
        };
        std::vector<TimeEvent> scheduledCallbacks_;
    };


}

#endif
