#pragma once

#include "srk/Core.h"
#include <atomic>
#include <thread>

namespace srk {
	class SRK_CORE_DLL EmptyLock {
	public:
		inline void SRK_CALL lock() {}
		inline bool SRK_CALL try_lock() { return false; }
		inline void SRK_CALL lock_shared() {}
		inline bool SRK_CALL try_lock_shared() { return false; }
		inline void SRK_CALL unlock() {}
		inline void SRK_CALL unlock_shared() {}
	};


	class SRK_CORE_DLL AtomicLock {
	public:
		AtomicLock() = default;
		AtomicLock(const AtomicLock&) = delete;
		AtomicLock(AtomicLock&&) = delete;
		AtomicLock& operator=(const AtomicLock&) = delete;
		AtomicLock& operator=(AtomicLock&&) = delete;

		inline void SRK_CALL lock() {
			_lock<false>();
		}

		inline bool SRK_CALL try_lock() {
			return _lock<true>();
		}

		inline void SRK_CALL unlock() {
			_flag.clear(std::memory_order::release);
		}

	private:
		std::atomic_flag _flag = ATOMIC_FLAG_INIT;

		template<bool Try>
		inline bool SRK_CALL _lock() {
			while (_flag.test_and_set(std::memory_order::acquire)) {
				if constexpr (Try) return false;
			}
			return true;
		}
	};


	class SRK_CORE_DLL RecursiveAtomicLock {
	public:
		RecursiveAtomicLock() = default;
		RecursiveAtomicLock(const RecursiveAtomicLock&) = delete;
		RecursiveAtomicLock(RecursiveAtomicLock&&) = delete;
		RecursiveAtomicLock& operator=(const RecursiveAtomicLock&) = delete;
		RecursiveAtomicLock& operator=(RecursiveAtomicLock&&) = delete;

		inline void SRK_CALL lock() {
			_lock<false>();
		}

		inline bool SRK_CALL try_lock() {
			return _lock<true>();
		}

		inline void SRK_CALL unlock() {
			if (!--_rc) _owner.store(std::thread::id(), std::memory_order::release);
		}

	private:
		uint32_t _rc = 0;
		std::atomic<std::thread::id> _owner = std::thread::id();

		template<bool Try>
		bool SRK_CALL _lock() {
			auto cur = std::this_thread::get_id();

			do {
				if (auto t = std::thread::id(); _owner.compare_exchange_weak(t, cur, std::memory_order::release, std::memory_order::relaxed) || t == cur) break;

				if constexpr (Try) return false;
			} while (true);

			++_rc;

			return true;
		}
	};


	class SRK_CORE_DLL SharedAtomicLock {
	public:
		SharedAtomicLock() = default;
		SharedAtomicLock(const SharedAtomicLock&) = delete;
		SharedAtomicLock(SharedAtomicLock&&) = delete;
		SharedAtomicLock& operator=(const SharedAtomicLock&) = delete;
		SharedAtomicLock& operator=(SharedAtomicLock&&) = delete;

		inline void SRK_CALL lock_shared() {
			_lockShared<false>();
		}

		inline bool SRK_CALL try_lock_shared() {
			return _lockShared<true>();
		}

		inline void SRK_CALL unlock_shared() {
			_rc.fetch_sub(1, std::memory_order::release);
		}

		inline void SRK_CALL lock() {
			_lock<false>();
		}

		inline bool SRK_CALL try_lock() {
			return _lock<true>();
		}

		inline void SRK_CALL unlock() {
			if (!--_wrc) {
				_writer.store(std::thread::id(), std::memory_order::release);
				_rc.store(1, std::memory_order::release);
			}
		}

	private:
		uint32_t _wrc = 0;
		std::atomic_uint32_t _rc = 1;
		std::atomic<std::thread::id> _writer = std::thread::id();

		template<bool Try>
		bool SRK_CALL _lockShared() {
			do {
				auto rc = _rc.load(std::memory_order::acquire);
				if (rc == 0) {
					if constexpr (Try) return false;
					continue;
				}

				if (_rc.compare_exchange_weak(rc, rc + 1)) break;

				if constexpr (Try) return false;
			} while (true);

			return true;
		}

		template<bool Try>
		inline bool SRK_CALL _lock() {
			auto cur = std::this_thread::get_id();

			do {
				if (_rc.load(std::memory_order::acquire) > 1) {
					if constexpr (Try) return false;
					continue;
				}

				uint32_t one = 1;
				if (!_rc.compare_exchange_weak(one, 0)) {
					if constexpr (Try) return false;
					continue;
				}

				if (auto t = std::thread::id(); _writer.compare_exchange_weak(t, cur, std::memory_order::release, std::memory_order::relaxed) || t == cur) break;

				if constexpr (Try) return false;
			} while (true);

			++_wrc;

			return true;
		}
	};
}