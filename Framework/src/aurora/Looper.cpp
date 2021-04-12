#include "Looper.h"
#include "aurora/Time.h"

namespace aurora {
	Looper::Looper(float64_t interval) :
		_eventDispatcher(new events::EventDispatcher<LooperEvent>()),
		_isRunning(std::make_shared<bool>(false)),
		_updatingCount(0),
		_updatingTimePoint(0),
		_updateTimeCompensationTimePoint(0),
		_updateTimeCompensationFrameCount(0) {
		setInterval(interval);
	}

	Looper::~Looper() {
		stop();
	}

	void Looper::run(bool restriction) {
		auto running = _isRunning;
		if (!*running) {
			*running = true;
			while (*running) tick(restriction);
		}
	}

	void Looper::tick(bool restriction) {
		auto t0 = Time::now();
		auto dt = _updatingCount++ == 0 ? 0 : (t0 - _updatingTimePoint);
		_updatingTimePoint = t0;

		_eventDispatcher->dispatchEvent(this, LooperEvent::TICKING, &dt);
		_eventDispatcher->dispatchEvent(this, LooperEvent::TICKED);

		if (restriction) {
			auto t1 = Time::now();
			auto phase = float64_t(t1 - t0);

			if (_updateTimeCompensationFrameCount) {
				if (auto t = _updateTimeCompensationFrameCount * _interval - (t0 - _updateTimeCompensationTimePoint); t > 0.) {
					t += _interval - phase;

					if (t > 0.) {
						++_updateTimeCompensationFrameCount;

						if (size_t st = t; st > 1) _sleep(st - 1);
					} else {
						_updateTimeCompensationFrameCount = 0;
					}
				} else {
					if (phase < _interval) {
						_updateTimeCompensationTimePoint = t0;
						_updateTimeCompensationFrameCount = 1;

						if (size_t st = _interval - phase; st > 1) _sleep(st - 1);
					} else {
						_updateTimeCompensationFrameCount = 0;
					}
				}
			} else {
				if (phase < _interval) {
					_updateTimeCompensationTimePoint = t0;
					_updateTimeCompensationFrameCount = 1;

					if (size_t st = _interval - phase; st > 1) _sleep(st - 1);
				}
			}
		} else {
			_updateTimeCompensationFrameCount = 0;
		}
	}

	void Looper::stop() {
		*_isRunning = false;
	}
}