#pragma once

#include "events/EventDispatcher.h"

AE_NS_BEGIN

template<typename T> class Rect;

AE_NS_END

AE_MODULE_GRAPHICS_NS_BEGIN

enum class Event : ui8 {
	UPDATE
};


class GraphicsModule : public AE_EVENT_NS::EventDispatcher<Event> {
public:
	GraphicsModule() : EventDispatcher(this) {
	}

	virtual bool AE_CALL createView(void* style, const i8* windowTitle, const Rect<i32>& rect, bool fullscreen, f64 fps) = 0;
	virtual void AE_CALL setFPS(f64 fps) = 0;
	virtual bool AE_CALL isWindowed() const = 0;
	virtual void AE_CALL toggleFullscreen() = 0;
	virtual void AE_CALL getViewRect(Rect<i32>& dst) const = 0;
	virtual void AE_CALL setViewRect(const Rect<i32>& rect) = 0;
	virtual void AE_CALL shutdown() = 0;
	virtual void AE_CALL run() = 0;

	virtual void AE_CALL beginRender() = 0;
	virtual void AE_CALL endRender() = 0;
	virtual void AE_CALL present() = 0;

	virtual void AE_CALL clear() = 0;
};

AE_MODULE_GRAPHICS_NS_END