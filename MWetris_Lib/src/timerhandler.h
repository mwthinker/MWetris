#ifndef MWETRIS_TIMEHANDLER_H
#define MWETRIS_TIMEHANDLER_H

#include <functional>
#include <vector>

namespace mwetris {

	class TimeHandler {
	public:
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

		// Update the internal time and trigger callbacks
		void update(double duration);

		// Schedule a callback function to be triggered after a given duration
		Key schedule(Callback callback, double delay);

		// Schedule a callback function to be repeatedly triggered at a specified interval
		Key scheduleRepeat(Callback callback, double interval, int maxNbr);

		/// @brief Remove callback associated with the key and return true if callback is found else false.
		/// @param key to the callback
		/// @return true if callback is found else false
		bool removeCallback(Key connection);

		double getCurrentTime() const;

		void clear();

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
