#ifndef MWETRIS_EVENTMANAGER_H
#define MWETRIS_EVENTMANAGER_H

#include <memory>
#include <functional>
#include <map>
#include <queue>

namespace mwetris {

	class Event {
	public:
		virtual ~Event() = default;
	};
	
	using EventPtr = std::shared_ptr<Event>;

	class TestEvent : public Event {
	public:
		TestEvent(int a) {
		}
	};

	class SenderId {
	public:
		friend class EventManager;

		SenderId() = default;

		bool operator<(SenderId other) const {
			return id_ < other.id_;
		}

		bool operator==(SenderId other) const {
			return id_ == other.id_;
		}

		bool operator!=(SenderId other) const {
			return id_ != other.id_;
		}

	private:
		SenderId(int id)
			: id_{id} {
		}
		
		int id_{};
	};

	class ReceiverId {
	public:
		friend class EventManager;

		ReceiverId() = default;

		bool operator<(ReceiverId other) const {
			return id_ < other.id_;
		}

		bool operator==(ReceiverId other) const {
			return id_ == other.id_;
		}

		bool operator!=(ReceiverId other) const {
			return id_ != other.id_;
		}

	private:
		ReceiverId(int id)
			: id_{id} {
		}

		int id_{};
	};

	class SubscriptionHandle {
	public:
		friend class EventManager;

		SubscriptionHandle() = default;

	private:
		SubscriptionHandle(SenderId senderId, ReceiverId receiverId)
			: senderId_{senderId}
			, receiverId_{receiverId} {
		}

		SenderId senderId_{};
		ReceiverId receiverId_{};
	};

	class EventManager {
	public:
		using Callback = std::function<void(EventPtr)>;

		EventManager() {}

		template <class Type, class... Args>
		void publish(SenderId senderId, Args&&... args);
		
		SubscriptionHandle subscribe(SenderId senderId, const Callback& callback);

		void unsubscribe(SubscriptionHandle handle);

		void runOne();

		void run();

		static SenderId generateSenderId();

	private:
		static ReceiverId generateReceiverId();

		struct MetaEvent {
			EventPtr event;
			SenderId id;
		};

		struct MetaCallback {
			Callback callback;
			ReceiverId receiverId;
		};

		void useFrontEvent();

		std::map<SenderId, std::vector<MetaCallback>> callbacks_;
		std::queue<MetaEvent> events_;
	};


	template <class Type, class... Args>
	void EventManager::publish(SenderId senderId, Args&&... args) {
		static_assert(std::is_base_of<Event, Type>::value,
			"Type must have Event as base class");
		
		events_.push(MetaEvent{std::make_shared<Type>(std::forward<Args>(args)...), senderId});
	}

}

#endif
