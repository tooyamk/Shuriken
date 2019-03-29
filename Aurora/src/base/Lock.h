#pragma once

#include "base/LowLevel.h"
#include <atomic>
#include <thread>

namespace aurora {
	class EmptyLock {
	public:
		inline void AE_CALL lock() {};
		inline void AE_CALL unlock() {};
	};


	class AtomicLock {
	public:
		AtomicLock() = default;
		AtomicLock(const AtomicLock&) = delete;
		AtomicLock& operator= (const AtomicLock&) = delete;

		inline void AE_CALL lock() {
			while (_flag.test_and_set(std::memory_order_acquire)) std::this_thread::yield();
		}

		inline void AE_CALL unlock() {
			_flag.clear(std::memory_order_release);
		}

	private:
		std::atomic_flag _flag = ATOMIC_FLAG_INIT;
	};


	class SpinAtomicLock {
	public:
		SpinAtomicLock() = default;
		SpinAtomicLock(const SpinAtomicLock&) = delete;
		SpinAtomicLock& operator= (const SpinAtomicLock&) = delete;

		inline void AE_CALL lock() {
			while (_flag.test_and_set(std::memory_order_acquire));
		}

		inline void AE_CALL unlock() {
			_flag.clear(std::memory_order_release);
		}

	private:
		std::atomic_flag _flag = ATOMIC_FLAG_INIT;
	};


	class RecursiveAtomicLock {
	public:
		RecursiveAtomicLock() = default;
		RecursiveAtomicLock(const RecursiveAtomicLock&) = delete;
		RecursiveAtomicLock& operator= (const RecursiveAtomicLock&) = delete;

		void AE_CALL lock() {
			auto cur = std::this_thread::get_id();

			/*
			do {
				ui32 v = 0;
				if (_rc.compare_exchange_weak(v, 1, std::memory_order_acquire, std::memory_order_relaxed)) {
					_owner.store(cur, std::memory_order_relaxed);
					break;
				}

				if (_owner.load(std::memory_order_relaxed) == cur) {
					++_rc;
					break;
				};

				std::this_thread::yield();
			} while (true);
			*/

			do {
				auto old = _lock.exchange(true, std::memory_order::memory_order_acquire);

				if (!old) {
					_owner.store(cur, std::memory_order::memory_order_relaxed);
					break;
				}

				if (old && _owner.load(std::memory_order::memory_order_relaxed) == cur) break;

				std::this_thread::yield();
			} while (true);

			++_rc;
		}

		void AE_CALL unlock() {
			/*
			if (_rc.load(std::memory_order_acquire) == 1) {
				_owner.store(std::thread::id(), std::memory_order_relaxed);
				_rc.store(0, std::memory_order_release);
			} else {
				_rc.fetch_sub(1, std::memory_order_relaxed);
			}
			*/
			if (--_rc == 0) {
				_owner.store(std::thread::id(), std::memory_order::memory_order_relaxed);
				_lock.store(false, std::memory_order::memory_order_release);
			}
		}

	private:
		//std::atomic_uint32_t _rc = 0;
		ui32 _rc = 0;
		std::atomic_bool _lock = false;
		std::atomic<std::thread::id> _owner;
	};


	class RecursiveSpinAtomicLock {
	public:
		RecursiveSpinAtomicLock() = default;
		RecursiveSpinAtomicLock(const RecursiveSpinAtomicLock&) = delete;
		RecursiveSpinAtomicLock& operator= (const RecursiveSpinAtomicLock&) = delete;

		void AE_CALL lock() {
			auto cur = std::this_thread::get_id();

			do {
				auto old = _lock.exchange(true, std::memory_order::memory_order_acquire);

				if (!old) {
					_owner.store(cur, std::memory_order::memory_order_relaxed);
					break;
				}

				if (old && _owner.load(std::memory_order::memory_order_relaxed) == cur) break;
			} while (true);

			++_rc;
		}
		inline void AE_CALL unlock() {
			if (--_rc == 0) {
				_owner.store(std::thread::id(), std::memory_order::memory_order_relaxed);
				_lock.store(false, std::memory_order::memory_order_release);
			}
		}

	private:
		ui32 _rc = 0;
		std::atomic_bool _lock = false;
		std::atomic<std::thread::id> _owner;
	};
}