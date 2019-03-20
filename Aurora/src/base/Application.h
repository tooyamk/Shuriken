#pragma once

#include "base/Aurora.h"

namespace aurora {
	namespace event {
		template<typename T> class IEventDispatcher;
	}

	class AE_TEMPLATE_DLL Application {
	public:
		enum class Event : ui8 {
			UPDATE
		};


		Application(f64 frameInterval);
		virtual ~Application();

		inline event::IEventDispatcher<Event>* AE_CALL getEventDispatcher() const {
			return _eventDispatcher;
		}

		void AE_CALL setEventDispatcher(event::IEventDispatcher<Event>* eventDispatcher);

		void AE_CALL run();
		void AE_CALL setFrameInterval(f64 frameInterval);
		void AE_CALL resetDeltaRecord();
		void AE_CALL update(bool autoSleep);
		void AE_CALL shutdown();

	protected:
		event::IEventDispatcher<Event>* _eventDispatcher;

		f64 _frameInterval; //microsecond
		i64 _time;
	};
}