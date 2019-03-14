#pragma once

#include <string>
#include "modules/graphics/GraphicsModule.h"
#include "math/Rect.h"

#include <GL/GL.h>
#pragma comment (lib, "opengl32.lib")

AE_MODULE_GRAPHICS_NS_BEGIN

class AE_MODULE_DLL GraphicsWinGL : public GraphicsModule {
public:
	GraphicsWinGL();
	virtual ~GraphicsWinGL();

	virtual void AE_CALL createView(void* style, const i8* windowTitle, const Rect<i32>& rect, bool fullscreen, f32 fps);
	virtual void AE_CALL setFPS(f32 fps);
	virtual bool AE_CALL isWindowed() const;
	virtual void AE_CALL toggleFullscreen();
	virtual void AE_CALL getViewRect(Rect<i32>& dst) const;
	virtual void AE_CALL setViewRect(const Rect<i32>& rect);
	virtual void AE_CALL shutdown();

private:
	bool _isWIndowed;
	std::wstring _className;
	f32 _tpf; //timePreFrame
	Rect<i32> _rect;

	HDC _dc;
	HGLRC _rc;

	bool AE_CALL _init(HWND hWnd);
	void AE_CALL _release();
};

AE_MODULE_GRAPHICS_NS_END

#ifdef AE_MODULE_EXPORTS
extern "C" AE_MODULE_DLL_EXPORT void* createModule() {
	return new AE_MODULE_GRAPHICS_NS::GraphicsWinGL();
}
#endif