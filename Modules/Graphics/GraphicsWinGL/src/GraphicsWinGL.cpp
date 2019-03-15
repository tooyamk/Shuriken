#include "GraphicsWinGL.h"
#include "utils/String.h"
#include <thread>

AE_MODULE_GRAPHICS_NS_BEGIN

GraphicsWinGL::GraphicsWinGL() :
	_isWIndowed(true),
	_tpf(0.f),
	_dc(nullptr),
	_rc(nullptr) {
}

GraphicsWinGL::~GraphicsWinGL() {
	_release();
}

void GraphicsWinGL::createView(void* style, const i8* windowTitle, const Rect<i32>& rect, bool fullscreen, f32 fps) {
	_rect.set(rect);
	_isWIndowed = fullscreen;
	setFPS(fps);

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
	if (!wnd.hInstance) wnd.hInstance = GetModuleHandle(nullptr);
	
	_className = wnd.lpszClassName;

	RegisterClassExW(&wnd);

	HWND hWnd = CreateWindowExW(0L, wnd.lpszClassName, String::UTF8ToUnicode(windowTitle).c_str(), WS_OVERLAPPEDWINDOW,
		_rect.left, _rect.top, _rect.getWidth(), _rect.getHeight(), GetDesktopWindow(), nullptr, wnd.hInstance, nullptr);
	//HWND hWnd = CreateWindowExW(0L, wnd.lpszClassName, String::UTF8ToUnicode(windowTitle).c_str(), WS_EX_TOPMOST, x, y, w, h, nullptr, nullptr, hIns, nullptr);
	if (hWnd && _init(hWnd)) {
		ShowWindow(hWnd, SW_SHOWDEFAULT);
		UpdateWindow(hWnd);

		MSG msg;

		ZeroMemory(&msg, sizeof(msg));

		//int aa = 0;

		while (msg.message != WM_QUIT) {
			if (PeekMessage(
				&msg,     // 存储消息的结构体指针
				nullptr,  // 窗口消息和线程消息都会被处理 
				0,        // 消息过滤最小值; 为0时返回所有可用信息
				0,        // 消息过滤最大值; 为0时返回所有可用信息
				PM_REMOVE // 指定消息如何处理; 消息在处理完后从队列中移除
			)) {
				TranslateMessage(&msg); // 变换虚拟键消息到字符消息，字符消息被发送到调用线程的消息队列
				DispatchMessage(&msg);  // 派发消息到窗口过程
			} else {
				//if (++aa == 120) {
					//PostQuitMessage(0);
				//}

				DWORD timeBegin = GetTickCount();                //循环开始的时间  

				//选择RC作为当前线程的RC
				wglMakeCurrent(_dc, _rc);

				//
				//在此使用OpenGL进行绘制
				//

				//交换当前缓冲区和后台缓冲区
				SwapBuffers(_dc);

				//取消当前线程选中的RC
				wglMakeCurrent(nullptr, nullptr);

				//DirectX_Update(hwnd);                          //directX循环
				//DirectX_Render(hwnd);                          //directX渲染
				f32 timePhase = f32(GetTickCount() - timeBegin); //循环耗费的时间
				if (timePhase < _tpf) {                           //循环耗费的时间<每帧的时间
					std::this_thread::sleep_for(std::chrono::milliseconds(DWORD(_tpf - timePhase))); //将剩余的时间等待
				}
			}
		}
	}

	_release();
}

void GraphicsWinGL::setFPS(f32 fps) {
	_tpf = fps <= 0.f ? 0.f : 1000.f / fps;
}

bool GraphicsWinGL::isWindowed() const {
	return _isWIndowed;
}

void GraphicsWinGL::toggleFullscreen() {
}

void GraphicsWinGL::getViewRect(Rect<i32>& dst) const {
	dst.set(_rect);
}

void GraphicsWinGL::setViewRect(const Rect<i32>& rect) {
	if (!_rect.isEqual(rect)) {
	}
}

void GraphicsWinGL::shutdown() {
	PostQuitMessage(0);
}

bool GraphicsWinGL::_init(HWND hWnd) {
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

	long style;
	style = GetWindowLong(hWnd, GWL_STYLE);

	DEVMODE dmScreenSettings;	 // Device Mode
	memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
	dmScreenSettings.dmSize = sizeof(dmScreenSettings);	 // Size Of The Devmode Structure
	dmScreenSettings.dmPelsWidth = GetSystemMetrics(SM_CXSCREEN);	 // Selected Screen Width
	dmScreenSettings.dmPelsHeight = GetSystemMetrics(SM_CYSCREEN);	 // Selected Screen Height
	dmScreenSettings.dmBitsPerPel = 32;	 // Selected Bits Per Pixel
	dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
	SetWindowLong(hWnd, GWL_STYLE, style&(~WS_OVERLAPPEDWINDOW));
	SetWindowPos(hWnd,
		HWND_TOPMOST, 0, 0,
		GetSystemMetrics(SM_CXSCREEN),
		GetSystemMetrics(SM_CYSCREEN),
		SWP_SHOWWINDOW);

	return true;
}

void GraphicsWinGL::_release() {
	if (_rc) {
		wglDeleteContext(_rc);
		_rc = nullptr;
	}
}

AE_MODULE_GRAPHICS_NS_END