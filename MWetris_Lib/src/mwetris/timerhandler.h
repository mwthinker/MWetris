#ifndef MWETRIS_TIMEHANDLER_H
#define MWETRIS_TIMEHANDLER_H

#include <functional>
#include <vector>

namespace mwetris {

	class TimeHandler {
	public:
		// Type definition for the callback function to be triggered by TimeHandler.
		using Callback = std::function<void()>;

		class Key {
		public:
			friend TimeHandler;

			Key() = default;

			bool operator==(Key key) const { return key.id_ == id_; };
			bool operator!=(Key key) const { return key.id_ != id_; };
		private:
			explicit Key(int id) : id_{id} {
			};

			int id_ = 0;
		};

		// Advance the internal time and trigger scheduled callbacks.
		void update(double duration);

		// Schedule a callback function to be triggered after a given duration.
		Key schedule(Callback callback, double delay);

		// Schedule a callback function to be repeatedly triggered at a specified interval.
		Key scheduleRepeat(Callback callback, double interval, int maxNbr);

		/// Remove the callback associated with the provided key.
		bool removeCallback(Key connection);

		// Get the internal time in seconds.
		double getCurrentTime() const;

		/// Clear all scheduled callbacks and reset the internal clock.
		void reset();

		/// @brief Has registered callback with the provided key.
		/// @return true if the callback is registered else false.
		bool hasKey(Key key) const;

	private:
		static int id_;

		double currentTime_ = 0.0;

		struct TimeEvent {
			int id;
			double eventTime;
			double interval;
			Callback callback;
			int maxNbr;
		};
		std::vector<TimeEvent> scheduledCallbacks_;
	};

}

#endif
