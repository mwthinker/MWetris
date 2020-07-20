#ifndef MWETRIS_GAME_TASKQUEUE_H
#define MWETRIS_GAME_TASKQUEUE_H

#include <functional>
#include <mutex>
#include <atomic>
#include <chrono>
#include <queue>

class TaskQueue {
public:
	using Task = std::function<void()>;

	TaskQueue()
		: thread_{&TaskQueue::excecute, this} {
	}

	~TaskQueue() {
		run_ = false;
		thread_.join();
	}

	void start() {
		pause_ = false;
	}

	void pause() {
		pause_ = true;
	}

	void excecute() {
		while (run_) {
			std::unique_lock<std::mutex> lock(mutex_);
			cv.wait(lock, [this]() { return !pause_;  });
			
			while (!tasks_.empty() && !pause_) {
				auto& task = tasks_.front();
				task();
				tasks_.pop();
			}
		}
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
	bool isWaiting() const {
		return !run_;
	}

	std::mutex mutex_;
	std::queue<Task> tasks_;
	std::condition_variable cv;
	std::thread thread_;
	std::atomic<bool> run_{false};
	std::atomic<bool> pause_{true};
};

#endif
