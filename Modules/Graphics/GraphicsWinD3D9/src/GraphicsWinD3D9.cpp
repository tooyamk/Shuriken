#include "GraphicsWinD3D9.h"
#include "utils/String.h"

namespace aurora::module::graphics{
	GraphicsWinD3D9::GraphicsWinD3D9() :
		_hWnd(nullptr),
		_hIns(nullptr),
		_d3d(nullptr),
		_d3dDevice(nullptr) {
		memset(&_d3dpp, 0, sizeof(_d3dpp));
		_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD; // 帧缓冲区交换方式; 可能是COPY可能是FLIP，由设备来确定适合当前情况的方式
	}

	GraphicsWinD3D9::~GraphicsWinD3D9() {
		_release();
	}

	bool GraphicsWinD3D9::createView(void* style, const i8* windowTitle, const Rect<i32>& rect, bool fullscreen) {
		_rect.set(rect);
		_d3dpp.Windowed = !fullscreen;
		_updateD3DParams();

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

		_hWnd = CreateWindowExW(0L, wnd.lpszClassName, String::UTF8ToUnicode(windowTitle).c_str(), WS_OVERLAPPEDWINDOW,
			_rect.left, _rect.top, _rect.getWidth(), _rect.getHeight(), GetDesktopWindow(), nullptr, _hIns, nullptr);
		//HWND hWnd = CreateWindowExW(0L, wnd.lpszClassName, String::UTF8ToUnicode(windowTitle).c_str(), WS_EX_TOPMOST, x, y, w, h, nullptr, nullptr, hIns, nullptr);
		if (_init(_hWnd)) {
			ShowWindow(_hWnd, SW_SHOWDEFAULT);
			UpdateWindow(_hWnd);

			return true;
		}

		return false;
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

	void GraphicsWinD3D9::_toggleFullscreen() {
		_d3dpp.Windowed = !_d3dpp.Windowed;
		if (!_d3dpp.Windowed) {
			RECT rect;
			GetWindowRect(_d3dpp.hDeviceWindow, &rect);
			_rect.set(rect.left, rect.top, rect.right, rect.bottom);
		}
		_updateD3DParams();
	}

	void GraphicsWinD3D9::beginRender() {
		_d3dDevice->BeginScene();
	}

	void GraphicsWinD3D9::endRender() {
		_d3dDevice->EndScene();
	}

	void GraphicsWinD3D9::present() {
		if (FAILED(_d3dDevice->Present(nullptr, nullptr, nullptr, nullptr))) {
			HRESULT hr = _d3dDevice->TestCooperativeLevel();
			if (SUCCEEDED(hr) || hr == D3DERR_DEVICENOTRESET) {
				HRESULT hr = _d3dDevice->Reset(&_d3dpp);
				if (SUCCEEDED(hr)) {
					//ok
				} else {
					//err
				}
			} else if (hr != D3DERR_DEVICELOST) {// Other error, Show error box
				//err
			}
		}
	}

	void GraphicsWinD3D9::clear() {
		_d3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(_d3dpp.Windowed ? 0xFF : 0, 0, 0), 1.0f, 0);
	}

	void GraphicsWinD3D9::_updateD3DParams() {
		if (_d3dpp.Windowed) {
			_d3dpp.FullScreen_RefreshRateInHz = 0;
		} else {
			_d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
			_d3dpp.BackBufferWidth = GetSystemMetrics(SM_CXSCREEN);
			_d3dpp.BackBufferHeight = GetSystemMetrics(SM_CYSCREEN);
		}
	}

	bool GraphicsWinD3D9::_init(HWND hWnd) {
		if (!hWnd) return false;
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

		_hWnd = nullptr;
	}
}