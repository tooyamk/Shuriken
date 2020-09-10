#pragma once

#include "aurora/Global.h"
#include <atomic>
#include <thread>

namespace aurora {
	class AE_CORE_DLL EmptyLock {
	public:
		inline void AE_CALL lock() {};
		inline void AE_CALL unlock() {};
	};


	template<bool GlobalBlock, bool Recursive>
	class AtomicLock {
	public:
		AtomicLock() = default;
		AtomicLock(const AtomicLock&) = delete;
		AtomicLock(AtomicLock&&) = delete;
		AtomicLock& operator=(const AtomicLock&) = delete;
		AtomicLock& operator=(AtomicLock&&) = delete;

		template<bool LocalBlock = GlobalBlock>
		inline bool AE_CALL lock() {
			while (_flag.test_and_set(std::memory_order_acquire)) {
				if constexpr (!LocalBlock) return false;
			}
			return true;
		}

		inline void AE_CALL unlock() {
			_flag.clear(std::memory_order_release);
		}

	private:
		std::atomic_flag _flag = ATOMIC_FLAG_INIT;
	};


	template<bool GlobalBlock>
	class AtomicLock<GlobalBlock, true> {
	public:
		AtomicLock() = default;
		AtomicLock(const AtomicLock&) = delete;
		AtomicLock(AtomicLock&&) = delete;
		AtomicLock& operator=(const AtomicLock&) = delete;
		AtomicLock& operator=(AtomicLock&&) = delete;

		template<bool LocalBlock = GlobalBlock>
		inline bool AE_CALL lock() {
			auto cur = std::this_thread::get_id();

			do {
				if (auto t = std::thread::id(); _owner.compare_exchange_weak(t, cur, std::memory_order_release, std::memory_order_relaxed) || t == cur) break;

				if constexpr (!LocalBlock) return false;
			} while (true);

			++_rc;

			return true;

			/*
			do {
				uint32_t zero = 0;
				if (_rc.compare_exchange_weak(zero, 1, std::memory_order_acquire, std::memory_order_relaxed)) {
					_owner.store(cur, std::memory_order_relaxed);
					break;
				}

				if (_owner.load(std::memory_order_acquire) == cur) {
					_rc.fetch_add(1, std::memory_order_relaxed);
					break;
				};

				if constexpr (!Spin) std::this_thread::yield();
			} while (true);
			*/

			/*
			do {
				auto old = _lock.exchange(true, std::memory_order::memory_order_acquire);
				if (!old) {
					_owner.store(cur, std::memory_order::memory_order_relaxed);
					break;
				}

				if (_owner.load(std::memory_order::memory_order_acquire) == cur) break;

				if constexpr (!Spin) std::this_thread::yield();
			} while (true);

			++_rc;
			*/
		}

		inline void AE_CALL unlock() {
			if (!--_rc) _owner.store(std::thread::id(), std::memory_order::memory_order_release);
			//_rc.fetch_sub(1, std::memory_order_relaxed);
			/*
			if (!--_rc) {
				_owner.store(std::thread::id(), std::memory_order::memory_order_relaxed);
				_lock.store(false, std::memory_order::memory_order_release);
			}
			*/
		}

	private:
		uint32_t _rc = 0;
		//std::atomic_bool _lock = false;
		//std::atomic_uint32_t _rc = 0;
		std::atomic<std::thread::id> _owner = std::thread::id();
	};


	template<bool ReadGlobalBlock, bool WriteGlobalBlock>
	class RWAtomicLock {
	public:
		RWAtomicLock() = default;
		RWAtomicLock(const RWAtomicLock&) = delete;
		RWAtomicLock(RWAtomicLock&&) = delete;
		RWAtomicLock& operator=(const RWAtomicLock&) = delete;
		RWAtomicLock& operator=(RWAtomicLock&&) = delete;

		template<bool LocalBlock = ReadGlobalBlock>
		inline bool AE_CALL readLock() {
			//auto cur = std::this_thread::get_id();

			do {
				uint32_t rc = _rc.load(std::memory_order_acquire);
				if (rc == 0) {
					if constexpr (!LocalBlock) return false;
					continue;
				}

				if (_rc.compare_exchange_weak(rc, rc + 1)) break;

				if constexpr (!LocalBlock) return false;
			} while (true);

			return true;
		}

		inline void AE_CALL readUnlock() {
			_rc.fetch_sub(1, std::memory_order_release);
		}

		template<bool LocalBlock = WriteGlobalBlock>
		inline bool AE_CALL writeLock() {
			auto cur = std::this_thread::get_id();

			do {
				if (_rc.load(std::memory_order_acquire) > 1) {
					if constexpr (!LocalBlock) return false;
					continue;
				}

				uint32_t one = 1;
				if (!_rc.compare_exchange_weak(one, 0)) {
					if constexpr (!LocalBlock) return false;
					continue;
				}

				if (auto t = std::thread::id(); _writer.compare_exchange_weak(t, cur, std::memory_order_release, std::memory_order_relaxed) || t == cur) break;

				if constexpr (!LocalBlock) return false;
			} while (true);

			++_wrc;

			return true;
		}

		inline void AE_CALL writeUnlock() {
			if (!--_wrc) {
				_writer.store(std::thread::id(), std::memory_order_release);
				_rc.store(1, std::memory_order_release);
			}
		}

	private:
		uint32_t _wrc = 0;
		std::atomic_uint32_t _rc = 1;
		std::atomic<std::thread::id> _writer = std::thread::id();
	};
}