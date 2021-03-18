#pragma once

#include "aurora/events/EventDispatcher.h"
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

namespace aurora {
	class AE_CORE_DLL TimeWheel : public std::enable_shared_from_this<TimeWheel> {
	public:
		class AE_CORE_DLL Timer : public Ref {
		public:
			Timer();
			virtual ~Timer();

			using OnTickFn = std::function<void(Timer& timer)>;

			inline void AE_CALL stop() {
				ref();
				{
					std::scoped_lock lock(_mutex);
					_stop();
				}
				Ref::unref(*this);
			}

			inline uint64_t AE_CALL getDelay() const {
				return _delay;
			}

			inline size_t AE_CALL getCount() const {
				return _count;
			}

			inline bool AE_CALL isStrict() const {
				return _isStrict;
			}

			inline bool AE_CALL isRunning() const {
				std::scoped_lock lck(_mutex);

				if (!_tickIDs.empty()) return true;

				if (_wheel.lock()) {
					return true;
				} else {
					return false;
				}
			}

			inline void AE_CALL doTick(uint64_t tickID) {
				std::scoped_lock lck(_mutex);

				if (auto itr = std::find(_tickIDs.begin(), _tickIDs.end(), tickID); itr != _tickIDs.end()) {
					_tickIDs.erase(itr);

					if (onTick) onTick(*this);
				}
			}

			OnTickFn onTick;

		private:
			bool _isStrict;
			mutable std::recursive_mutex _mutex;
			//AtomicLock<true, true> _lock;
			std::weak_ptr<TimeWheel> _wheel;
			size_t _count;
			uint64_t _delay;

			size_t _runningCount;
			size_t _runningCycle;
			uint64_t _runningDelay;
			size_t _slotIndex;
			std::list<uint64_t> _tickIDs;
			Timer* _prev;
			Timer* _next;

			inline void AE_CALL _stop() {
				if (auto p = _wheel.lock(); p) p->stopTimer(*this);
				_tickIDs.clear();
			}

			friend TimeWheel;
		};


		TimeWheel(uint64_t interval, size_t numSlots);

		inline uint64_t AE_CALL getInterval() const {
			return _interval;
		}

		void AE_CALL startTimer(Timer& timer, uint64_t delay, size_t count, bool strict);
		IntrusivePtr<Timer> AE_CALL startTimer(uint64_t delay, size_t count, bool strict, const Timer::OnTickFn& fn);

		inline void AE_CALL stopTimer(Timer& timer) {
			std::scoped_lock lck(_mutex);

			_stopTimer(timer);
		}

		template<InvocableAnyOfResult<std::tuple<void, bool>, Timer&, uint64_t> Fn>
		void AE_CALL tick(uint64_t elapsed, Fn&& fn) {
			{
				std::scoped_lock lck(_mutex);

				_tickingLastTime = elapsed;

				if (_slotElapsed) {
					auto last = _interval - _slotElapsed;
					if (_tickingLastTime >= last) {
						_tickingElapsed = last;
						_slots[_curSlotIndex].tick(*this, true, _tickingElapsed, fn);
						_tickingElapsed = 0;

						_slotElapsed = 0;
						_tickingLastTime -= last;
						_rollPointer();
					}
				}
			}

			if (_tickingLastTime) {
				while (_tickingLastTime >= _interval) {
					std::scoped_lock lck(_mutex);

					_tickingElapsed = _interval;
					_slots[_curSlotIndex].tick(*this, true, _tickingElapsed, fn);
					_tickingElapsed = 0;

					_tickingLastTime -= _interval;
					_rollPointer();
				}

				if (_tickingLastTime) {
					std::scoped_lock lck(_mutex);

					_tickingElapsed = _tickingLastTime;
					_slots[_curSlotIndex].tick(*this, false, _tickingElapsed, fn);
					_tickingElapsed = 0;

					_slotElapsed += _tickingLastTime;
					_tickingLastTime = 0;
				}
			}
		}

	private:
		struct Slot {
			Slot() : head(nullptr), tail(nullptr) {}

			Timer* head;
			Timer* tail;

			void AE_CALL add(Timer* t) {
				if (head) {
					t->_prev = tail;
					tail->_next = t;
					tail = t;
				} else {
					head = t;
					tail = t;
				}
			}

			void AE_CALL remove(Timer*& tickingTimer, Timer* t) {
				if (tickingTimer == t) tickingTimer = t->_next;

				if (head == t) {
					head = t->_next;
					if (!head) tail = nullptr;
				} else if (tail == t) {
					tail = t->_prev;
					tail->_next = nullptr;
				} else {
					auto prev = t->_prev;
					auto next = t->_next;
					prev->_next = next;
					next->_prev = prev;
				}

				t->_prev = nullptr;
				t->_next = nullptr;
			}

			template<typename Fn>
			void AE_CALL tick(TimeWheel& wheel, bool skip, uint64_t elapsed, Fn&& fn) {
				if (head) {
					auto& tickingTimer = wheel._tickingTimer;
					auto t = head;
					do {
						tickingTimer = t;
						if (t->_runningCycle) {
							if (skip) --t->_runningCycle;
						} else if (t->_runningDelay <= elapsed) {
							IntrusivePtr<Timer> p = t;
							++t->_runningCount;
							auto e = t->_runningDelay;

							auto id = ++wheel._tickIDGenerator;

							{
								std::scoped_lock lck(t->_mutex);
								t->_tickIDs.emplace_back(id);
							}

							if (t->_count && t->_count <= t->_runningCount) {
								this->remove(tickingTimer, t);
								t->_wheel.reset();
								Ref::unref(*t);
								fn(*t, id);
							} else {
								fn(*t, id);
								if (t == tickingTimer) wheel._repeatTimer(*t, e);
							}

							t = tickingTimer;

							continue;
						} else {
							t->_runningDelay -= elapsed;
						}

						t = t->_next;
					} while (t);
				}
			}
		};


		std::recursive_mutex _mutex;
		std::vector<Slot> _slots;
		size_t _numSlots;
		size_t _curSlotIndex;
		uint64_t _tickingElapsed;
		uint64_t _slotElapsed;
		uint64_t _interval;
		uint64_t _cycle;

		uint64_t _tickIDGenerator;

		uint64_t _tickingLastTime;
		Timer* _tickingTimer;

		inline void AE_CALL _rollPointer() {
			if (++_curSlotIndex == _numSlots) _curSlotIndex = 0;
		}

		void AE_CALL _addTimer(Timer& timer, uint64_t delay, uint64_t elapsed, bool strict);
		void AE_CALL _repeatTimer(Timer& timer, uint64_t elapsed);
		void AE_CALL _stopTimer(Timer& timer);

		friend Slot;
	};
}