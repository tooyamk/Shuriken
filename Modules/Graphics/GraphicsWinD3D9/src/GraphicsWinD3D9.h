#pragma once

#include <string>
#include "modules/graphics/GraphicsModule.h"
#include "math/Rect.h"

#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

namespace aurora::module::graphics {
	class AE_MODULE_DLL GraphicsWinD3D9 : public GraphicsModule {
	public:
		GraphicsWinD3D9();
		virtual ~GraphicsWinD3D9();

		virtual bool AE_CALL createView(void* style, const i8* windowTitle, const Rect<i32>& rect, bool fullscreen);
		virtual bool AE_CALL isWindowed() const;
		virtual void AE_CALL toggleFullscreen();
		virtual void AE_CALL getViewRect(Rect<i32>& dst) const;
		virtual void AE_CALL setViewRect(const Rect<i32>& rect);

		virtual void AE_CALL beginRender();
		virtual void AE_CALL endRender();
		virtual void AE_CALL present();

		virtual void AE_CALL clear();

	private:
		std::wstring _className;
		Rect<i32> _rect;
		HWND _hWnd;

		HINSTANCE _hIns;
		PDIRECT3D9 _d3d; // D3D对象
		PDIRECT3DDEVICE9 _d3dDevice; // D3D设备对象
		D3DPRESENT_PARAMETERS _d3dpp;

		bool AE_CALL _init(HWND hWnd);
		void AE_CALL _release();
		void AE_CALL _toggleFullscreen();
		void AE_CALL _updateD3DParams();
	};
}

#ifdef AE_MODULE_EXPORTS
extern "C" AE_MODULE_DLL_EXPORT void* createModule() {
	return new AE_MODULE_GRAPHICS_NS::GraphicsWinD3D9();
}
#endif