#pragma once

#include "srk/Core.h"

namespace srk::lockfree {
	enum class RingQueueMode : uint8_t {
		SPSC = 0b00
	};

	template<typename T, RingQueueMode Mode>
	class RingQueue;

	template<typename T>
	class RingQueue<T, RingQueueMode::SPSC> {
	public:
		static constexpr RingQueueMode MODE = RingQueueMode::SPSC;

		RingQueue(size_t capacity) :
			_capacity(capacity),
			_mem((std::remove_cvref_t<T>*)malloc(_capacity * sizeof(T))),
			_pushIndex(0),
			_popIndex(0),
			_count(0) {
		}

		~RingQueue() {
			while (auto cnt = _count.load()) {
				(_mem + _popIndex)->~T();
				if (++_popIndex == _capacity) _popIndex = 0;
			}

			free(_mem);
		}

		inline bool push(const T& item) {
			return _push(item);
		}

		inline bool push(T&& item) {
			return _push(std::forward<T>(item));
		}

		bool pop(T& out) {
			auto cnt = _count.load();
			if (cnt == 0) return false;

			auto tmp = _mem + _popIndex;
			if constexpr (std::is_move_assignable_v<T>) {
				out = std::move(*tmp);
			} else {
				out = *tmp;
			}

			tmp->~T();
			_count.fetch_sub(1);

			if (++_popIndex == _capacity) _popIndex = 0;

			return true;
		}

	private:
		size_t _capacity;
		std::remove_cvref_t<T>* _mem;

		size_t _pushIndex;
		size_t _popIndex;
		std::atomic_size_t _count;

		template<typename I>
		bool _push(I&& item) {
			if (_count.load() >= _capacity) return false;

			new(_mem + _pushIndex) T(std::forward<I>(item));
			_count.fetch_add(1);

			if (++_pushIndex == _capacity) _pushIndex = 0;

			return true;
		}
	};
}