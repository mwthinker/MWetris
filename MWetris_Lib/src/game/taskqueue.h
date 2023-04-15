#ifndef MWETRIS_GAME_TASKQUEUE_H
#define MWETRIS_GAME_TASKQUEUE_H

#include <functional>
#include <mutex>
#include <atomic>
#include <chrono>
#include <queue>
#include <thread>

class TaskQueue {
public:
	using Task = std::function<void()>;

	TaskQueue()
		: thread_{&TaskQueue::excecute, this} {
	}

	void start() {
		pause_ = false;
	}

	void pause() {
		pause_ = true;
	}

	void push(const Task& item) {
		std::lock_guard<std::mutex> lock(mutex_);
		tasks_.push(item);
	}

	void push(Task&& item) {
		std::lock_guard<std::mutex> lock(mutex_);
		tasks_.push(std::move(item));
	}

	void clear() {
		std::lock_guard<std::mutex> lock(mutex_);
		while (!tasks_.empty()) {
			tasks_.pop();
		}
	}

private:
	void excecute(std::stop_token st) {
		while (!st.stop_requested()) {
			std::unique_lock<std::mutex> lock(mutex_);
			cv.wait(lock, [this]() { return !pause_;  });

			while (!tasks_.empty() && !pause_) {
				auto& task = tasks_.front();
				task();
				tasks_.pop();
			}
		}
	}

	std::mutex mutex_;
	std::atomic<bool> pause_ = true;
	std::queue<Task> tasks_;
	std::condition_variable cv;
	std::jthread thread_;
};

#endif
