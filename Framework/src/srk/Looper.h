#pragma once

#include "srk/events/EventDispatcher.h"
#include <thread>

#if SRK_OS == SRK_OS_WINDOWS
#	include "mmsystem.h"
#endif

namespace srk {
	enum class LooperEvent : uint8_t {
		TICKING,
		TICKED
	};


	class SRK_FW_DLL Looper : public Ref {
	public:
		Looper(float64_t interval);
		virtual ~Looper();

		inline IntrusivePtr<events::IEventDispatcher<LooperEvent>> SRK_CALL getEventDispatcher() {
			return _eventDispatcher;
		}

		inline bool SRK_CALL isRunning() const {
			return *_isRunning;
		}

		inline float64_t SRK_CALL getInterval() const {
			return _interval;
		}
		inline void SRK_CALL setInterval(float64_t interval) {
			_interval = interval < 0. ? 0. : interval;
			_internalInterval = _interval * 1000000000.;
		}

		inline void SRK_CALL resetDeltaRecord() {
			_updatingCount = 0;
			_updateTimeCompensationFrameCount = 0;
		}

		void SRK_CALL run(bool restriction);
		void SRK_CALL tick(bool restriction);
		void SRK_CALL stop();

	protected:
		IntrusivePtr<events::IEventDispatcher<LooperEvent>> _eventDispatcher;

		std::shared_ptr<bool> _isRunning;
		float64_t _interval;
		float64_t _internalInterval;
		size_t _updatingCount;
		int64_t _updatingTimePoint;
		int64_t _updateTimeCompensationTimePoint;
		size_t _updateTimeCompensationFrameCount;

		void SRK_CALL _sleep(size_t nanoseconds);
	};
}