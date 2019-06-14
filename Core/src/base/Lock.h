#pragma once

#include "base/LowLevel.h"
#include <atomic>
#include <thread>

namespace aurora {
	class AE_DLL EmptyLock {
	public:
		inline void AE_CALL lock() {};
		inline void AE_CALL unlock() {};
	};


	template<bool Spin, bool Recursive>
	class AE_TEMPLATE_DLL AtomicLock {
	public:
		AtomicLock() = default;
		AtomicLock(const AtomicLock&) = delete;
		AtomicLock& operator=(const AtomicLock&) = delete;

		inline void AE_CALL lock() {
			while (_flag.test_and_set(std::memory_order_acquire)) {
				if constexpr (!Spin) std::this_thread::yield();
			}
		}

		inline void AE_CALL unlock() {
			_flag.clear(std::memory_order_release);
		}

	private:
		std::atomic_flag _flag = ATOMIC_FLAG_INIT;
	};


	template<bool Spin>
	class AE_TEMPLATE_DLL AtomicLock<Spin, true> {
	public:
		AtomicLock() = default;
		AtomicLock(const AtomicLock&) = delete;
		AtomicLock& operator=(const AtomicLock&) = delete;

		inline void AE_CALL lock() {
			auto cur = std::this_thread::get_id();

			do {
				if (auto t = std::thread::id(); _owner.compare_exchange_weak(t, cur, std::memory_order_release, std::memory_order_relaxed) || t == cur) break;

				if constexpr (!Spin) std::this_thread::yield();
			} while (true);

			++_rc;
		}

		inline void AE_CALL unlock() {
			if (!--_rc) _owner.store(std::thread::id(), std::memory_order::memory_order_release);
		}

	private:
		ui32 _rc = 0;
		std::atomic<std::thread::id> _owner = std::thread::id();
	};


	template<bool ReadSpin, bool WriteSpin>
	class AE_TEMPLATE_DLL RWAtomicLock {
	public:
		RWAtomicLock() = default;
		RWAtomicLock(const RWAtomicLock&) = delete;
		RWAtomicLock& operator=(const RWAtomicLock&) = delete;

		inline void AE_CALL readLock() {
			do {
				uint32_t rc = _rc.load(std::memory_order_acquire);
				if (rc == 0) {
					if constexpr (!ReadSpin) std::this_thread::yield();
					continue;
				}

				if (_rc.compare_exchange_weak(rc, rc + 1)) break;

				if constexpr (!ReadSpin) std::this_thread::yield();
			} while (true);
		}

		inline void AE_CALL readUnlock() {
			_rc.fetch_sub(1, std::memory_order_release);
		}

		inline void AE_CALL writeLock() {
			auto cur = std::this_thread::get_id();

			do {
				if (_rc.load(std::memory_order_acquire) > 1) {
					if constexpr (!ReadSpin) std::this_thread::yield();
					continue;
				}

				uint32_t one;
				if (!_rc.compare_exchange_weak(one, 0)) {
					if constexpr (!WriteSpin) std::this_thread::yield();
					continue;
				}

				if (auto t = std::thread::id(); _writer.compare_exchange_weak(t, cur, std::memory_order_release, std::memory_order_relaxed) || t == cur) break;

				if constexpr (!WriteSpin) std::this_thread::yield();
			} while (true);

			++_wrc;
		}

		inline void AE_CALL writeUnlock() {
			if (!--_wrc) {
				_writer.store(std::thread::id(), std::memory_order_release);
				_rc.store(1, std::memory_order_release);
			}
		}

	private:
		ui32 _wrc = 0;
		std::atomic_uint32_t _rc = 1;
		std::atomic<std::thread::id> _writer = std::thread::id();
	};
}