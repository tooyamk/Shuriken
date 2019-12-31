#pragma once

#include "events/EventDispatcher.h"
#include <functional>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <vector>

namespace aurora {
	class AE_DLL TimeWheel : public std::enable_shared_from_this<TimeWheel> {
	public:
		enum class TimeWheelEvent : uint8_t {
			TRIGGER
		};


		class AE_DLL Timer : public Ref {
		public:
			enum class TimerEvent : uint8_t {
				TICK
			};


			Timer();
			~Timer();

			inline void AE_CALL stop() {
				ref();
				{
					std::scoped_lock lock(_mutex);
					_stop();
				}
				unref();
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
				return !_wheel.expired();
			}

			inline events::IEventDispatcher<TimerEvent>& AE_CALL getEventDispatcher() {
				return _eventDispatcher;
			}

			inline void AE_CALL doTick() {
				_eventDispatcher.dispatchEvent(this, TimerEvent::TICK);
			}

		private:
			bool _isStrict;
			std::recursive_mutex _mutex;
			//AtomicLock<true, true> _lock;
			std::weak_ptr<TimeWheel> _wheel;
			size_t _count;
			uint64_t _delay;

			size_t _runningCount;
			size_t _runningCycle;
			uint64_t _runningDelay;
			size_t _slotIndex;
			Timer* _prev;
			Timer* _next;

			events::EventDispatcher<TimerEvent> _eventDispatcher;

			inline void AE_CALL _stop() {
				if (auto p = _wheel.lock(); p) p->stopTimer(*this);
			}

			friend TimeWheel;
		};


		TimeWheel(uint64_t interval, size_t numSlots);

		inline events::IEventDispatcher<TimeWheelEvent>& AE_CALL getEventDispatcher() {
			return _eventDispatcher;
		}

		inline uint64_t AE_CALL getInterval() const {
			return _interval;
		}

		void AE_CALL startTimer(Timer& timer, uint64_t delay, size_t count, bool strict);

		inline void AE_CALL stopTimer(Timer& timer) {
			std::shared_lock lck(_mutex);

			_stopTimer(timer);
		}

		void AE_CALL tick(uint64_t elapsed);

	private:
		struct Slot {
			Slot() : head(nullptr), tail(nullptr) {}

			Timer* head;
			Timer* tail;

			std::mutex _mutex;

			void AE_CALL add(Timer* t) {
				std::scoped_lock lck(_mutex);

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

			void AE_CALL tick(TimeWheel& wheel, bool skip, uint64_t elapsed) {
				if (head) {
					auto& tickingTimer = wheel._tickingTimer;
					auto t = head;
					do {
						tickingTimer = t;
						if (t->_runningCycle) {
							if (skip) --t->_runningCycle;
						} else if (t->_runningDelay <= elapsed) {
							RefPtr<Timer> p = t;
							++t->_runningCount;
							auto e = t->_runningDelay;
							if (t->_count && t->_count <= t->_runningCount) {
								this->remove(tickingTimer, t);
								t->_wheel.reset();
								t->unref();
								wheel._eventDispatcher.dispatchEvent(&wheel, TimeWheelEvent::TRIGGER, t);
							} else {
								wheel._eventDispatcher.dispatchEvent(&wheel, TimeWheelEvent::TRIGGER, t);
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


		std::shared_mutex _mutex;
		std::vector<Slot> _slots;
		size_t _numSlots;
		size_t _curSlotIndex;
		uint64_t _tickingElapsed;
		uint64_t _slotElapsed;
		uint64_t _interval;
		uint64_t _cycle;

		uint64_t _tickingLastTime;
		Timer* _tickingTimer;

		events::EventDispatcher<TimeWheelEvent> _eventDispatcher;

		inline void AE_CALL _rollPointer() {
			if (++_curSlotIndex == _numSlots) _curSlotIndex = 0;
		}

		void AE_CALL _addTimer(Timer& timer, uint64_t delay, uint64_t elapsed, bool strict);
		void AE_CALL _repeatTimer(Timer& timer, uint64_t elapsed);
		void AE_CALL _stopTimer(Timer& timer);

		friend Slot;
	};
}