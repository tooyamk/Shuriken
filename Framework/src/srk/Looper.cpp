#include "Looper.h"
#include "srk/Time.h"

namespace srk {
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
		auto t0 = Time::now<std::chrono::nanoseconds, std::chrono::steady_clock>();
		auto dt = _updatingCount++ == 0 ? 0. : (t0 - _updatingTimePoint) / 1000000000.;
		_updatingTimePoint = t0;

		IntrusivePtr self = *this;
		_eventDispatcher->dispatchEvent((void*)this, LooperEvent::TICKING, &dt);
		_eventDispatcher->dispatchEvent((void*)this, LooperEvent::TICKED);

		if (restriction) {
			auto t1 = Time::now<std::chrono::nanoseconds, std::chrono::steady_clock>();
			auto phase = float64_t(t1 - t0);

			if (_updateTimeCompensationFrameCount) {
				if (auto t = _updateTimeCompensationFrameCount * _internalInterval - (t0 - _updateTimeCompensationTimePoint); t > 0.) {
					t += _internalInterval - phase;

					if (t > 0.) {
						++_updateTimeCompensationFrameCount;

						_sleep(t);
					} else {
						_updateTimeCompensationFrameCount = 0;
					}
				} else {
					if (phase < _internalInterval) {
						_updateTimeCompensationTimePoint = t0;
						_updateTimeCompensationFrameCount = 1;

						_sleep(_internalInterval - phase);
					} else {
						_updateTimeCompensationFrameCount = 0;
					}
				}
			} else {
				if (phase < _internalInterval) {
					_updateTimeCompensationTimePoint = t0;
					_updateTimeCompensationFrameCount = 1;

					_sleep(_internalInterval - phase);
				}
			}
		} else {
			_updateTimeCompensationFrameCount = 0;
		}
	}

	void Looper::_sleep(size_t nanoseconds) {
		if (nanoseconds > 0) {
			auto t0 = Time::now<std::chrono::nanoseconds, std::chrono::steady_clock>();

#if SRK_OS == SRK_OS_WINDOWS
			auto ms = nanoseconds / 1000000ull;
			if (ms > 1)
			{
				timeBeginPeriod(1);
				std::this_thread::sleep_for(std::chrono::milliseconds(ms - 1));
				timeEndPeriod(1);
			}
			do {
				auto t1 = Time::now<std::chrono::nanoseconds, std::chrono::steady_clock>();
				if (t1 - t0 >= nanoseconds) break;
			} while (true);
#else
			do {
				std::this_thread::yield();
				auto t1 = Time::now<std::chrono::nanoseconds, std::chrono::steady_clock>();
				if (t1 - t0 >= nanoseconds) break;
			} while (true);
#endif
		}
	}

	void Looper::stop() {
		*_isRunning = false;
	}
}