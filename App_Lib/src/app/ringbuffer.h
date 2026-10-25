#ifndef APP_RINGBUFFER_H
#define APP_RINGBUFFER_H

#include <array>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <utility>

namespace app {

	template<typename T, int Capacity>
	requires (Capacity > 0) && std::default_initializable<T> && std::movable<T>
	class RingBuffer {
	public:
		[[nodiscard]] constexpr bool tryPush(T value) {
			if (full()) {
				return false;
			}

			values_[static_cast<std::size_t>(tail_)] = std::move(value);
			tail_ = (tail_ + 1) % capacityValue_;
			++size_;
			return true;
		}

		[[nodiscard]] constexpr T& front() {
			assert(!empty());
			return values_[static_cast<std::size_t>(head_)];
		}

		[[nodiscard]] constexpr const T& front() const {
			assert(!empty());
			return values_[static_cast<std::size_t>(head_)];
		}

		constexpr void pop() {
			assert(!empty());
			values_[static_cast<std::size_t>(head_)] = T{};
			head_ = (head_ + 1) % capacityValue_;
			--size_;
		}

		constexpr void clear() {
			while (!empty()) {
				pop();
			}
			head_ = 0;
			tail_ = 0;
		}

		[[nodiscard]] constexpr bool empty() const {
			return size_ == 0;
		}

		[[nodiscard]] constexpr bool full() const {
			return size_ == capacityValue_;
		}

		[[nodiscard]] constexpr int size() const {
			return size_;
		}

		[[nodiscard]] static constexpr int capacity() {
			return Capacity;
		}

	private:
		static constexpr int capacityValue_ = Capacity;
		static constexpr std::size_t storageCapacity_ = static_cast<std::size_t>(Capacity);
		std::array<T, storageCapacity_> values_ = {};
		int head_ = 0;
		int tail_ = 0;
		int size_ = 0;
	};

}

#endif
