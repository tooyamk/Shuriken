#pragma once

#include <string>
#include "modules/graphics/GraphicsModule.h"
#include "math/Rect.h"

#include "GL/glew.h"
#pragma comment (lib, "glew.lib")

#include <GL/GL.h>
#pragma comment (lib, "opengl32.lib")

namespace aurora::module::graphics{
	class AE_MODULE_DLL GraphicsWinWGL : public GraphicsModule {
	public:
		GraphicsWinWGL();
		virtual ~GraphicsWinWGL();

		virtual bool AE_CALL createView(void* style, const i8* windowTitle, const Rect<i32>& rect, bool fullscreen) override;
		virtual bool AE_CALL isWindowed() const override;
		virtual void AE_CALL toggleFullscreen() override;
		virtual void AE_CALL getViewRect(Rect<i32>& dst) const override;
		virtual void AE_CALL setViewRect(const Rect<i32>& rect) override;
		
		virtual void AE_CALL beginRender() override;
		virtual void AE_CALL endRender() override;
		virtual void AE_CALL present() override;

		virtual void AE_CALL clear() override;

	private:
		bool _isWindowed;
		std::wstring _className;
		Rect<i32> _rect;

		ui32 _dwStyle;

		HINSTANCE _hIns;
		HWND _hWnd;

		HDC _dc;
		HGLRC _rc;

		bool AE_CALL _init(HWND hWnd);
		void AE_CALL _release();
		void AE_CALL _updateWndParams();
	};
}
#ifdef AE_MODULE_EXPORTS
extern "C" AE_MODULE_DLL_EXPORT void* createModule() {
	return new aurora::module::graphics::GraphicsWinWGL();
}
#endif