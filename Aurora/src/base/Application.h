#pragma once

#include "events/EventDispatcher.h"

AE_NS_BEGIN

enum class ApplicationEvent : ui8 {
	ENTER_FRAME
};


class AE_TEMPLATE_DLL Application : public AE_EVENT_NS::EventDispatcher<ApplicationEvent> {
public:
	Application(f64 frameInterval);

	void AE_CALL run();
	void AE_CALL setFrameInterval(f64 frameInterval);
	void AE_CALL resetDeltaRecord();
	void AE_CALL update(bool autoSleep);
	void AE_CALL shutdown();

protected:
	f64 _frameInterval; //microsecond
	i64 _time;
};

AE_NS_END