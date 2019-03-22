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
		std::wstring _className;
		Rect<i32> _rect;

		HINSTANCE _hIns;
		HWND _hWnd;
		
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
	return new aurora::module::graphics::GraphicsWinD3D9();
}
#endif