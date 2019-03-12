#pragma once

#include "modules/Module.h"

AE_NS_BEGIN

template<typename T> class Rect;

class AE_DLL GraphicsModule {
public:
	virtual void AE_CALL createView(void* style, const i8* windowTitle, const Rect<i32>& rect, bool fullscreen, f32 fps) = 0;
	virtual void AE_CALL setFPS(f32 fps) = 0;
	virtual bool AE_CALL isWindowed() const = 0;
	virtual void AE_CALL toggleFullscreen() = 0;
	virtual void AE_CALL getViewRect(Rect<i32>& dst) const = 0;
	virtual void AE_CALL setViewRect(const Rect<i32>& rect) = 0;
	virtual void AE_CALL shutdown() = 0;
};

AE_NS_END