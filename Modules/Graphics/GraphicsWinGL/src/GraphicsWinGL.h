#pragma once

#include "modules/GraphicsModule.h"
#include "math/Rect.h"

#include <GL/GL.h>
#pragma comment (lib, "opengl32.lib")

AE_NS_BEGIN

class AE_MODULE_DLL GraphicsWinGL : public GraphicsModule {
public:
	GraphicsWinGL();

	virtual void AE_CALL createView(void* style, const i8* windowTitle, const Rect<i32>& rect, bool fullscreen, f32 fps);
	virtual void AE_CALL setFPS(f32 fps);
	virtual bool AE_CALL isWindowed() const;
	virtual void AE_CALL toggleFullscreen();
	virtual void AE_CALL getViewRect(Rect<i32>& dst) const;
	virtual void AE_CALL setViewRect(const Rect<i32>& rect);
	virtual void AE_CALL shutdown();

private:
	f32 _tpf; //timePreFrame
	Rect<i32> _rect;
};

AE_NS_END

#ifdef AE_MODULE_EXPORTS
extern "C" AE_MODULE_DLL_EXPORT void* createModule() {
	return new AE_NS::GraphicsWinGL();
}
#endif