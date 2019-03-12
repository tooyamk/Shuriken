#include "GraphicsWinDX.h"
#include "base/Console.h"

#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

AE_NS_BEGIN

void GraphicsWinDX::createView(void* style, const i8* windowTitle, i32 x, i32 y, i32 w, i32 h) {
	HINSTANCE hIns = GetModuleHandle(nullptr);

	WNDCLASSEXA& wnd = *(WNDCLASSEXA*)style;
	wnd.cbSize = sizeof(WNDCLASSEXA);
	wnd.lpfnWndProc = [](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
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
	wnd.cbClsExtra = 0;
	wnd.cbWndExtra = 0;
	wnd.hInstance = hIns;

	RegisterClassExA(&wnd);

	HWND hWnd = CreateWindowExA(0L, wnd.lpszClassName, windowTitle, WS_OVERLAPPEDWINDOW, x, y, w, h, GetDesktopWindow(), nullptr, hIns, nullptr);
	if (hWnd) {

	}
}

AE_NS_END