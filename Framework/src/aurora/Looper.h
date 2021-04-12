#pragma once

#include "aurora/events/EventDispatcher.h"
#include <thread>

#if AE_OS == AE_OS_WIN
#	include "mmsystem.h"
#endif

namespace aurora {
	enum class LooperEvent : uint8_t {
		TICKING,
		TICKED
	};


	class AE_FW_DLL Looper : public Ref {
	public:
		Looper(float64_t interval);
		virtual ~Looper();

		inline IntrusivePtr<events::IEventDispatcher<LooperEvent>> AE_CALL getEventDispatcher() {
			return _eventDispatcher;
		}

		inline bool AE_CALL isRunning() const {
			return *_isRunning;
		}

		inline float64_t AE_CALL getInterval() const {
			return _interval;
		}
		inline void AE_CALL setInterval(float64_t interval) {
			_interval = interval < 0. ? 0. : interval;
		}

		inline void AE_CALL resetDeltaRecord() {
			_updatingCount = 0;
			_updateTimeCompensationFrameCount = 0;
		}

		void AE_CALL run(bool restriction);
		void AE_CALL tick(bool restriction);
		void AE_CALL stop();

	protected:
		IntrusivePtr<events::IEventDispatcher<LooperEvent>> _eventDispatcher;

		std::shared_ptr<bool> _isRunning;
		float64_t _interval;
		size_t _updatingCount;
		int64_t _updatingTimePoint;
		int64_t _updateTimeCompensationTimePoint;
		size_t _updateTimeCompensationFrameCount;

		inline void AE_CALL _sleep(size_t milliseconds) {
#if AE_OS == AE_OS_WIN
			timeBeginPeriod(1);
#endif
			std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
#if AE_OS == AE_OS_WIN
			timeEndPeriod(1);
#endif
		}
	};
}