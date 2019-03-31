#include "Graphics.h"
#include "base/Application.h"
#include "Module.h"
#include "utils/String.h"

namespace aurora::modules::graphics_win_glew {
	Graphics::Graphics(Graphics::CREATE_PARAMS_REF params) :
		_app(params.application->ref<Application>()),
		_dc(nullptr),
		_rc(nullptr) {
		_app->ref();
	}

	Graphics::~Graphics() {
		_release();
		Ref::setNull(_app);
	}

	bool Graphics::createDevice() {
		if (_dc) return false;

		HWND hWnd = _app->$Win$_getHWND();
		if (!hWnd) return false;

		_dc = GetDC(hWnd);
		if (_dc) {
			PIXELFORMATDESCRIPTOR pfd;
			pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
			pfd.nVersion = 1;
			pfd.dwFlags =
				PFD_DRAW_TO_WINDOW |            //格式支持窗口
				PFD_SUPPORT_OPENGL |            //格式必须支持OpenGL
				PFD_DOUBLEBUFFER;               //必须支持双缓冲
			pfd.iPixelType = PFD_TYPE_RGBA;     //申请RGBA 格式
			pfd.cColorBits = 24;                //选定色彩深度
			pfd.cRedBits = 0;                   //忽略RGBA
			pfd.cRedShift = 0;
			pfd.cGreenBits = 0;
			pfd.cGreenShift = 0;
			pfd.cBlueBits = 0;
			pfd.cBlueShift = 0;
			pfd.cAlphaBits = 0;
			pfd.cAlphaShift = 0;
			pfd.cAccumBits = 0;                 //无累加缓存
			pfd.cAccumRedBits = 0;              //忽略聚集位
			pfd.cAccumGreenBits = 0;
			pfd.cAccumBlueBits = 0;
			pfd.cAccumAlphaBits = 0;
			pfd.cDepthBits = 32;                //32位Z-缓存(深度缓存)
			pfd.cStencilBits = 0;               //无蒙板缓存
			pfd.cAuxBuffers = 0;                //无辅助缓存
			pfd.iLayerType = PFD_MAIN_PLANE;    //主绘图层
			pfd.bReserved = 0;                  //Reserved
			pfd.dwLayerMask = 0;                //忽略层遮罩
			pfd.dwVisibleMask = 0;
			pfd.dwDamageMask = 0;

			auto pf = ChoosePixelFormat(_dc, &pfd);
			if (!SetPixelFormat(_dc, pf, &pfd)) return false;
			
			_rc = wglCreateContext(_dc);
			if (_rc) {
				wglMakeCurrent(_dc, _rc);
				if (glewInit() == GLEW_OK) {
					glFrontFace(GL_CW);

					return true;
				}
			}
		}

		_release();
		return false;

		/*
		long style = GetWindowLong(hWnd, GWL_STYLE);

		DEVMODE dmScreenSettings;	 // Device Mode
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);	 // Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth =GetSystemMetrics(SM_CXSCREEN);	 // Selected Screen Width
		dmScreenSettings.dmPelsHeight =  GetSystemMetrics(SM_CYSCREEN);	 // Selected Screen Height
		dmScreenSettings.dmBitsPerPel = 32;	 // Selected Bits Per Pixel
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		//if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL) {
			SetWindowLong(hWnd, GWL_STYLE, style&(~WS_OVERLAPPEDWINDOW));
		//}
		SetWindowPos(hWnd,
			HWND_NOTOPMOST,
			0,
			0,
			GetSystemMetrics(SM_CXSCREEN),
			GetSystemMetrics(SM_CYSCREEN),
			SWP_SHOWWINDOW);
			*/
	}

	GraphicsModule::VertexBuffer* Graphics::createVertexBuffer() {
		return new graphics_win_glew::VertexBuffer(*this);
	}

	GraphicsModule::Program* Graphics::createProgram() {
		return new graphics_win_glew::Program(*this);
	}

	void Graphics::beginRender() {
		wglMakeCurrent(_dc, _rc);

		i32 w, h;
		_app->getInnerSize(w, h);
		glViewport(0, 0, w, h);
	}

	void Graphics::endRender() {
		//交换当前缓冲区和后台缓冲区
		SwapBuffers(_dc);

		if (wglGetCurrentContext() == _rc) wglMakeCurrent(nullptr, nullptr);
	}

	void Graphics::present() {
	}

	void Graphics::clear() {
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void Graphics::_release() {
		if (wglGetCurrentContext() == _rc) wglMakeCurrent(nullptr, nullptr);

		if (_rc) {
			wglDeleteContext(_rc);
			_rc = nullptr;
		}

		if (_dc) {
			ReleaseDC(_app->$Win$_getHWND(), _dc);
			_dc = nullptr;
		}

		if (_app) {
			_app->unref();
			_app = nullptr;
		}
	}
}