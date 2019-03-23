#include "Graphics.h"
#include "utils/String.h"
#include "VertexBuffer.h"
#include <thread>

namespace aurora::modules::graphics::win::glew {
	Graphics::Graphics() :
		_isWindowed(true),
		_hIns(nullptr),
		_hWnd(nullptr),
		_dwStyle(0),
		_dc(nullptr),
		_rc(nullptr) {
	}

	Graphics::~Graphics() {
		_release();
	}

	bool Graphics::createView(void* style, const i8* windowTitle, const Rect<i32>& windowedRect, bool fullscreen) {
		_windowedRect.set(windowedRect);
		_isWindowed = !fullscreen;
		_updateWndParams();

		WNDCLASSEXW wnd = *(WNDCLASSEXW*)style;
		if (!wnd.cbSize) wnd.cbSize = sizeof(WNDCLASSEXW);
		if (!wnd.lpfnWndProc) wnd.lpfnWndProc = [](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
			switch (msg) {
			case WM_DESTROY:
				PostQuitMessage(0);
				return (LRESULT)0;
				break;
			default:
				break;
			}
			
			return DefWindowProc(hWnd, msg, wParam, lParam);
		};
		if (wnd.hInstance) {
			_hIns = wnd.hInstance;
		} else {
			_hIns = GetModuleHandle(nullptr);
			wnd.hInstance = _hIns;
		}
		_className = wnd.lpszClassName;

		RegisterClassExW(&wnd);
		_hWnd = CreateWindowExW(0L, wnd.lpszClassName, String::Utf8ToUnicode(windowTitle).c_str(), _dwStyle,
			_curRect.left, _curRect.top, _curRect.getWidth(), _curRect.getHeight(), GetDesktopWindow(), nullptr, _hIns, nullptr);
		//HWND hWnd = CreateWindowExW(0L, wnd.lpszClassName, String::UTF8ToUnicode(windowTitle).c_str(), WS_EX_TOPMOST, x, y, w, h, nullptr, nullptr, hIns, nullptr);
		if (_init(_hWnd)) {
			ShowWindow(_hWnd, SW_SHOWDEFAULT);
			UpdateWindow(_hWnd);

			return true;
		}

		return false;
	}

	bool Graphics::isWindowed() const {
		return _isWindowed;
	}

	void Graphics::toggleFullscreen() {
		_isWindowed = !_isWindowed;
		if (!_isWindowed) _updateWindowedRect();

		_updateWndParams();
		_changeWnd();
	}

	void Graphics::getWindowedRect(Rect<i32>& dst) const {
		if (_isWindowed) _updateWindowedRect();
		dst.set(_windowedRect);
	}

	void Graphics::setWindowedRect(const Rect<i32>& rect) {
		if (!_windowedRect.isEqual(rect)) {
			_windowedRect.set(rect);
			if (_isWindowed) {
				_updateWndParams();
				_changeWnd();
			}
		}
	}

	aurora::modules::graphics::VertexBuffer* Graphics::createVertexBuffer() {
		ui32 buf = 0;
		glGenBuffers(1, &buf);

		if (buf) {
			this->ref();
			return new VertexBuffer(this);
		}

		return nullptr;
	}

	void Graphics::beginRender() {
		wglMakeCurrent(_dc, _rc);
	}

	void Graphics::endRender() {
		//交换当前缓冲区和后台缓冲区
		SwapBuffers(_dc);

		//取消当前线程选中的RC
		wglMakeCurrent(nullptr, nullptr);
	}

	void Graphics::present() {
	}

	void Graphics::clear() {
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void Graphics::_updateWndParams() {
		if (_isWindowed) {
			_dwStyle |= WS_OVERLAPPEDWINDOW;
			_curRect.set(_windowedRect);
		} else {
			_dwStyle &= ~WS_OVERLAPPEDWINDOW;
			_curRect.set(0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
		}
	}

	void Graphics::_changeWnd() {
		SetWindowLong(_hWnd, GWL_STYLE, _dwStyle);
		SetWindowPos(_hWnd, HWND_NOTOPMOST, _curRect.left, _curRect.top, _curRect.getWidth(), _curRect.getHeight(), SWP_SHOWWINDOW);
	}

	void Graphics::_updateWindowedRect() const {
		RECT rect;
		GetWindowRect(_hWnd, &rect);
		_windowedRect.set(rect.left, rect.top, rect.right, rect.bottom);
	}

	bool Graphics::_init(HWND hWnd) {
		if (!hWnd) return false;

		_dc = GetDC(hWnd);

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

		int pf = ChoosePixelFormat(_dc, &pfd);
		SetPixelFormat(_dc, pf, &pfd);

		_rc = wglCreateContext(_dc);

		wglMakeCurrent(_dc, _rc);
		if (glewInit() != GLEW_OK) return false;

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

		return true;
	}

	void Graphics::_release() {
		wglMakeCurrent(nullptr, nullptr);

		if (_rc) {
			wglDeleteContext(_rc);
			_rc = nullptr;
		}

		if (_dc) {
			ReleaseDC(_hWnd, _dc);
			_dc = nullptr;
		}

		if (_hWnd) {
			DestroyWindow(_hWnd);
			_hWnd = nullptr;
		}

		if (_hIns) {
			UnregisterClass(_className.c_str(), _hIns);
			_hIns = nullptr;
		}
	}
}