#include <functional>
#include <mutex>
#include <queue>

class TaskQueue {
public:
	using Task = std::function<void()>;

	TaskQueue() = default;
	~TaskQueue() = default;

	TaskQueue(const TaskQueue&) = delete;
	TaskQueue(TaskQueue&&) = delete;
	TaskQueue& operator=(TaskQueue&&) = delete;
	TaskQueue& operator=(TaskQueue&) = delete;	

	void excecute() {
		std::unique_lock<std::mutex> lock(mutex_);
		while (!tasks_.empty()) {
			auto& task = tasks_.front();
			task();
			tasks_.pop();
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
	std::mutex mutex_;
	std::queue<Task> tasks_;
};
