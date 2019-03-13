#include "GraphicsWinGL.h"
#include "utils/String.h"

AE_MODULE_GRAPHICS_NS_BEGIN

void GraphicsWinGL::createView(void* style, const i8* windowTitle, const Rect<i32>& rect, bool fullscreen, f32 fps) {
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
				if (timePhase < _tpf) {              //循环耗费的时间<每帧的时间
					sleepms(DWORD(_tpf - timePhase)); //将剩余的时间等待
				}

				GetTickCount();
			}
		}
	}

	_release();
}

AE_MODULE_GRAPHICS_NS_END