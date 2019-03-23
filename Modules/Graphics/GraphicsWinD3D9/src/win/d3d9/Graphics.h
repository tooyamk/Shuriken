#pragma once

#include <string>
#include "modules/graphics/GraphicsModule.h"
#include "math/Rect.h"

#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

namespace aurora::modules::graphics::win::d3d9 {
	class AE_MODULE_DLL Graphics : public GraphicsModule {
	public:
		Graphics();
		virtual ~Graphics();

		virtual bool AE_CALL createView(void* style, const i8* windowTitle, const Rect<i32>& windowedRect, bool fullscreen) override;
		virtual bool AE_CALL isWindowed() const override;
		virtual void AE_CALL toggleFullscreen() override;
		virtual void AE_CALL getWindowedRect(Rect<i32>& dst) const override;
		virtual void AE_CALL setWindowedRect(const Rect<i32>& rect) override;

		virtual void AE_CALL beginRender() override;
		virtual void AE_CALL endRender() override;
		virtual void AE_CALL present() override;

		virtual void AE_CALL clear() override;

	private:
		std::wstring _className;
		mutable Rect<i32> _windowedRect;

		HINSTANCE _hIns;
		HWND _hWnd;
		
		PDIRECT3D9 _d3d; // D3D对象
		PDIRECT3DDEVICE9 _d3dDevice; // D3D设备对象
		D3DPRESENT_PARAMETERS _d3dpp;

		bool AE_CALL _init(HWND hWnd);
		void AE_CALL _release();
		void AE_CALL _toggleFullscreen();
		void AE_CALL _updateD3DParams();
		void AE_CALL _updateWindowedRect() const;
	};
}

#ifdef AE_MODULE_EXPORTS
extern "C" AE_MODULE_DLL_EXPORT void* createModule() {
	return new aurora::modules::graphics::win::d3d9::Graphics();
}
#endif