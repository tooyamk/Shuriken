#include "TimeWheel.h"

namespace aurora {
	TimeWheel::Timer::Timer() :
		_isStrict(false),
		_delay(0),
		_count(0),
		_prev(nullptr),
		_next(nullptr) {
	}

	TimeWheel::Timer::~Timer() {
	}


	TimeWheel::TimeWheel(uint64_t interval, size_t numSlots) :
		_slots(numSlots),
		_numSlots(numSlots),
		_curSlotIndex(0),
		_tickingElapsed(0),
		_slotElapsed(0),
		_interval(interval),
		_cycle(interval * numSlots),
		_tickingLastTime(0),
		_tickingTimer(nullptr) {
	}

	void TimeWheel::startTimer(TimeWheel::Timer& timer, uint64_t delay, size_t count, bool strict) {
		std::scoped_lock lock(timer._mutex);
		
		timer.ref();
		timer._stop();
		timer._wheel = weak_from_this();
		timer._isStrict = strict;
		timer._delay = delay;
		timer._count = count;
		timer._runningCount = 0;

		std::scoped_lock lock2(_mutex);

		_addTimer(timer, delay, _tickingElapsed, strict);
	}

	void TimeWheel::_repeatTimer(TimeWheel::Timer& timer, uint64_t elapsed) {
		auto oldIdx = timer._slotIndex;

		_slots[oldIdx].remove(_tickingTimer, &timer);

		_addTimer(timer, timer._delay, elapsed, timer._isStrict);

		if (!_tickingTimer && timer._slotIndex == oldIdx && !timer._runningCycle) {
			_tickingTimer = &timer;
		}
	}

	void TimeWheel::_addTimer(TimeWheel::Timer& timer, uint64_t delay, uint64_t elapsed, bool strict) {
		if (!strict) delay += _tickingLastTime;

		size_t c = delay < _cycle ? 0 : delay / _cycle;
		delay += _curSlotIndex * _interval + _slotElapsed + elapsed;
		delay %= _cycle;

		size_t i = delay / _interval;
		i %= _numSlots;

		if (!c && i == _curSlotIndex) delay -= _slotElapsed;

		timer._slotIndex = i;
		timer._runningCycle = c;
		timer._runningDelay = delay % _interval;

		_slots[i].add(&timer);
	}

	void TimeWheel::_stopTimer(TimeWheel::Timer& timer) {
		std::scoped_lock lck(timer._mutex);

		if (&*timer._wheel.lock() == this) {
			{
				std::unique_lock lck(_mutex);

				_slots[timer._slotIndex].remove(_tickingTimer, &timer);
			}

			timer._wheel.reset();
		}
	}

	void TimeWheel::tick(uint64_t elapsed) {
		{
			std::scoped_lock lck(_mutex);

			_tickingLastTime = elapsed;

			if (_slotElapsed) {
				auto last = _interval - _slotElapsed;
				if (_tickingLastTime >= last) {
					_tickingElapsed = last;
					_slots[_curSlotIndex].tick(*this, true, _tickingElapsed);
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
				_slots[_curSlotIndex].tick(*this, true, _tickingElapsed);
				_tickingElapsed = 0;

				_tickingLastTime -= _interval;
				_rollPointer();
			}

			if (_tickingLastTime) {
				std::scoped_lock lck(_mutex);

				_tickingElapsed = _tickingLastTime;
				_slots[_curSlotIndex].tick(*this, false, _tickingElapsed);
				_tickingElapsed = 0;

				_slotElapsed += _tickingLastTime;
				_tickingLastTime = 0;
			}
		}
	}
}