#include "GraphicsWinD3D9.h"
#include "utils/String.h"

AE_MODULE_GRAPHICS_NS_BEGIN

GraphicsWinD3D9::GraphicsWinD3D9() :
	_hIns(nullptr),
	_d3d(nullptr),
	_d3dDevice(nullptr),
	_tpf(0.f) {
	ZeroMemory(&_d3dpp, sizeof(_d3dpp));
	_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD; // 帧缓冲区交换方式; 可能是COPY可能是FLIP，由设备来确定适合当前情况的方式
}

GraphicsWinD3D9::~GraphicsWinD3D9() {
	_release();
}

void GraphicsWinD3D9::createView(void* style, const i8* windowTitle, const Rect<i32>& rect, bool fullscreen, f32 fps) {
	_rect.set(rect);
	_d3dpp.Windowed = !fullscreen;
	_d3dpp.FullScreen_RefreshRateInHz = fullscreen ? D3DPRESENT_RATE_DEFAULT : 0;
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
	if (wnd.hInstance) {
		_hIns = wnd.hInstance;
	} else {
		_hIns = GetModuleHandle(nullptr);
		wnd.hInstance = _hIns;
	}
	_className = wnd.lpszClassName;

	RegisterClassExW(&wnd);

	HWND hWnd = CreateWindowExW(0L, wnd.lpszClassName, String::UTF8ToUnicode(windowTitle).c_str(), WS_OVERLAPPEDWINDOW, 
		_rect.left, _rect.top, _rect.getWidth(), _rect.getHeight(), GetDesktopWindow(), nullptr, _hIns, nullptr);
	//HWND hWnd = CreateWindowExW(0L, wnd.lpszClassName, String::UTF8ToUnicode(windowTitle).c_str(), WS_EX_TOPMOST, x, y, w, h, nullptr, nullptr, hIns, nullptr);
	if (hWnd && _init(hWnd)) {
		ShowWindow(hWnd, SW_SHOWDEFAULT);
		UpdateWindow(hWnd);

		MSG msg;

		ZeroMemory(&msg, sizeof(msg));

		//int aa = 0;

		while (msg.message != WM_QUIT) {
			if (PeekMessage(
				&msg, // 存储消息的结构体指针
				nullptr, // 窗口消息和线程消息都会被处理 
				0, // 消息过滤最小值; 为0时返回所有可用信息
				0, // 消息过滤最大值; 为0时返回所有可用信息
				PM_REMOVE // 指定消息如何处理; 消息在处理完后从队列中移除
			)) {
				TranslateMessage(&msg); // 变换虚拟键消息到字符消息，字符消息被发送到调用线程的消息队列
				DispatchMessage(&msg); // 派发消息到窗口过程
			} else {
				//if (++aa == 120) {
					//PostQuitMessage(0);
				//}

				DWORD timeBegin = GetTickCount();             //循环开始的时间  

				_d3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(_d3dpp.Windowed ? 0xFF : 0, 0, 0), 1.0f, 0);
				_d3dDevice->BeginScene();
				// 3D图形数据
				_d3dDevice->EndScene();

				// 显示backbuffer内容到屏幕
				_d3dDevice->Present(nullptr, nullptr, nullptr, nullptr);

				//DirectX_Update(hwnd);                         //directX循环
				//DirectX_Render(hwnd);                         //directX渲染
				f32 timePhase = f32(GetTickCount() - timeBegin); //循环耗费的时间
				if (timePhase < _tpf){              //循环耗费的时间<每帧的时间
					sleepms(DWORD(_tpf - timePhase)); //将剩余的时间等待
				}

				GetTickCount();
			}
		}
	}

	_release();
}

void GraphicsWinD3D9::setFPS(f32 fps) {
	_tpf = fps <= 0.f ? 0.f : 1000.f / fps;
}

bool GraphicsWinD3D9::isWindowed() const {
	return _d3dpp.Windowed;
}

void GraphicsWinD3D9::toggleFullscreen() {
	//OnDeviceLost();

	_toggleFullscreen();
	
	HRESULT rst = _d3dDevice->Reset(&_d3dpp);
	if (!SUCCEEDED(rst)) {
		_toggleFullscreen();
		_d3dDevice->Reset(&_d3dpp);
	}

	if (_d3dpp.Windowed) {
		SetWindowPos(_d3dpp.hDeviceWindow, HWND_TOP, _rect.left, _rect.top, _rect.getWidth(), _rect.getHeight(), SWP_NOZORDER | SWP_DRAWFRAME | SWP_SHOWWINDOW);
	}

	//OnDeviceRestore();
}

void GraphicsWinD3D9::getViewRect(Rect<i32>& dst) const {
	dst.set(_rect);
}

void GraphicsWinD3D9::setViewRect(const Rect<i32>& rect) {
	if (!_rect.isEqual(rect)) {
		if (_d3dpp.Windowed) {
			if (_rect.getWidth() == rect.getWidth() && _rect.getWidth() == rect.getHeight()) {
				_rect.set(rect);
				SetWindowPos(_d3dpp.hDeviceWindow, HWND_TOP, _rect.left, _rect.top, _rect.getWidth(), _rect.getHeight(), SWP_NOZORDER | SWP_DRAWFRAME | SWP_SHOWWINDOW);
			} else {
				//OnDeviceLost();

				HRESULT rst = _d3dDevice->Reset(&_d3dpp);
				if (SUCCEEDED(rst)) {
					_rect.set(rect);
					SetWindowPos(_d3dpp.hDeviceWindow, HWND_TOP, _rect.left, _rect.top, _rect.getWidth(), _rect.getHeight(), SWP_NOZORDER | SWP_DRAWFRAME | SWP_SHOWWINDOW);
				}

				//OnDeviceRestore();
			}
		} else {
			_rect.set(rect);
		}
	}
}

void GraphicsWinD3D9::shutdown() {
	PostQuitMessage(0);
}

void GraphicsWinD3D9::_toggleFullscreen() {
	_d3dpp.Windowed = !_d3dpp.Windowed;
	if (_d3dpp.Windowed) {
		_d3dpp.FullScreen_RefreshRateInHz = 0;
	} else {
		RECT rect;
		GetWindowRect(_d3dpp.hDeviceWindow, &rect);
		_rect.set(rect.left, rect.top, rect.right, rect.bottom);

		_d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
		_d3dpp.BackBufferWidth = GetSystemMetrics(SM_CXSCREEN);
		_d3dpp.BackBufferHeight = GetSystemMetrics(SM_CYSCREEN);
	}
}

bool GraphicsWinD3D9::_init(HWND hWnd) {
	// 显示模式 （以像素为单位的屏幕宽高，刷新频率，surface formt）
	D3DDISPLAYMODE displayMode;

	// 创建D3D对象 获取主显卡硬件信息  最先被创建，最后被释放
	_d3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (!_d3d) return false;

	// 获取显示模式
	if (FAILED(_d3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT/*查询主显卡*/, &displayMode))) return false;

	_d3dpp.hDeviceWindow = hWnd;
	_d3dpp.BackBufferFormat = displayMode.Format;  // D3DFMT_X8R8G8B8  表示为32位RGB像素格式 每种颜色用一个字节表示
	// 创建D3D设备对象 ---- 代表显卡
	if (FAILED(_d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &_d3dpp, &_d3dDevice))) return false;

	return true;
}

void GraphicsWinD3D9::_release() {
	if (_d3dDevice) {
		_d3dDevice->Release();
		_d3dDevice = nullptr;
	}
	if (_d3d) {
		_d3d->Release();
		_d3d = nullptr;
	}
	if (_hIns) {
		UnregisterClassW(_className.c_str(), _hIns);
		_hIns = nullptr;
	}
}

AE_MODULE_GRAPHICS_NS_END