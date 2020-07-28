#include "eventmanager.h"

namespace mwetris{

	SubscriptionHandle EventManager::subscribe(SenderId senderId, const Callback& callback) {
		auto receiverId = generateReceiverId();
		callbacks_[senderId].push_back(MetaCallback{callback, receiverId});
		return SubscriptionHandle{senderId, receiverId};
	}

	void EventManager::unsubscribe(SubscriptionHandle handle) {
		if (auto it = callbacks_.find(handle.senderId_); it != callbacks_.end()) {
			auto& callbacks = it->second;

			auto it2 = std::remove_if(callbacks.begin(), callbacks.end(), [&](const MetaCallback& meta) {
				return meta.receiverId == handle.receiverId_;
			});
			if (it2 != callbacks.end()) {
				callbacks.erase(it2, callbacks.end());
			}
		}
	}

	void EventManager::runOne() {
		if (!events_.empty()) {
			useFrontEvent();
		}
	}

	void EventManager::run() {
		while (!events_.empty()) {
			useFrontEvent();
		}
	}

	SenderId EventManager::generateSenderId() {
		static int id = 0;
		return SenderId{++id};
	}

	ReceiverId EventManager::generateReceiverId() {
		static int id = 0;
		return ReceiverId{++id};
	}

	void EventManager::useFrontEvent() {
		const auto& meta = events_.front();
		const auto& callbacks = callbacks_[meta.id];
		for (auto& metaCallback : callbacks) {
			metaCallback.callback(meta.event);
		}
		events_.pop();
	}

}
