#include "Application.h"
#include "aurora/String.h"

namespace aurora {
	Application::Application(const std::string_view& appId) :
		_appId(appId),
		_isClosing(false),
#if AE_OS == AE_OS_WIN
		_win({ nullptr, nullptr, {0, 0, 0, 0}, nullptr }),
#elif AE_OS == AE_OS_LINUX
		_linux({ false, nullptr, 0 }),
#endif
		_isFullscreen(false) {
	}

	Application::~Application() {
#if AE_OS == AE_OS_WIN
		if (_win.hWnd) {
			DestroyWindow(_win.hWnd);
			_win.hWnd = nullptr;
		}

		if (_win.bkBrush) {
			DeleteObject(_win.bkBrush);
			_win.bkBrush = nullptr;
		}

		if (_win.hIns) {
			auto appIdW = String::Utf8ToUnicode(_appId);
			UnregisterClassW(appIdW.data(), _win.hIns);
			_win.hIns = nullptr;
		}
#elif AE_OS == AE_OS_LINUX
		if (_linux.window) {
			XDestroyWindow(_linux.dis, _linux.window);
			_linux.window = 0;
		}

		if (_linux.dis) {
			XCloseDisplay(_linux.dis);
			_linux.dis = nullptr;
		}
#endif
	}

	bool Application::createWindow(const Style& style, const std::string_view& title, const Box2i32ui32& clientRect, bool fullscreen) {
		_isFullscreen = fullscreen;
		_style = style;

#if AE_OS == AE_OS_WIN
		_win.hIns = GetModuleHandle(nullptr);
		_win.bkBrush = CreateSolidBrush(RGB(style.backgroundColor[0], style.backgroundColor[1], style.backgroundColor[2]));

		WNDCLASSEXW wnd;
		memset(&wnd, 0, sizeof(wnd));
		wnd.cbSize = sizeof(wnd);
		wnd.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wnd.lpfnWndProc = Application::_wndProc;
		wnd.cbClsExtra = 0;
		wnd.cbWndExtra = 0;
		wnd.hInstance = _win.hIns;
		wnd.hIcon = nullptr;//LoadIcon(NULL, IDI_APPLICATION);
		wnd.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wnd.hbrBackground = _win.bkBrush;
		wnd.lpszMenuName = nullptr;//NULL;
		auto appIdW = String::Utf8ToUnicode(_appId);
		wnd.lpszClassName = appIdW.data();
		wnd.hIconSm = nullptr;

		RegisterClassExW(&wnd);

		_adjustWindowRect(clientRect, _windowRect);
		_updateWindowRectValue();

		_win.hWnd = CreateWindowExW(_getWindowExStyle(), wnd.lpszClassName, String::Utf8ToUnicode(title).data(), _getWindowStyle(),
			_wndRect.pos[0], _wndRect.pos[1], _wndRect.size[0], _wndRect.size[1],
			GetDesktopWindow(), nullptr, _win.hIns, nullptr);
		SetWindowLongPtr(_win.hWnd, GWLP_USERDATA, (LONG_PTR)this);
		if (_win.hWnd) GetClientRect(_win.hWnd, &_win.lastWndClientRect);

		//DeleteObject(bkBrush);

		return _win.hWnd;
#elif AE_OS == AE_OS_LINUX
		if (!_linux.dis) _linux.dis = XOpenDisplay(nullptr);
		if (!_linux.dis) return false;

		auto s = DefaultScreen(_linux.dis);

		XSetWindowAttributes attr = { 0 };
		attr.border_pixel = 0;
		attr.background_pixel = style.backgroundColor[0] < 16 | style.backgroundColor[1] < 8 | style.backgroundColor[2];

		_linux.window = XCreateWindow(_linux.dis, RootWindow(_linux.dis, s),
			windowedRect.pos[0], windowedRect.pos[1], windowedRect.size[0], windowedRect.size[1], 1,
			DefaultDepth(_linux.dis, 0), InputOutput, DefaultVisual(_linux.dis, 0), CWBackPixel, &attr);

		_linux.MOTIF_WM_HINTS = XInternAtom(_linux.dis, "_MOTIF_WM_HINTS", False);

		{
			// auto hints = XAllocSizeHints();
			// hints->flags = PMinSize | PMaxSize;
			// hints->min_width = 10;
			// hints->min_height = 10;
			// hints->max_width = 100;
			// hints->max_height = 100;
			// XSetWMNormalHints(_linux.dis, _linux.window, hints);
			// XFree(hints);
		}

		// Atom window_type = XInternAtom(_linux.dis, "_NET_WM_WINDOW_TYPE", False);
		// int64_t value = XInternAtom(_linux.dis, "_NET_WM_WINDOW_TYPE_NORMAL", False);
		// XChangeProperty(_linux.dis, _linux.window, window_type, XA_ATOM, 32, PropModeReplace, (uint8_t*)&value, 1);

		{
			MwmHints hints = { 0 };

			hints.flags = MwmHints::MWM_HINTS_FUNCTIONS | MwmHints::MWM_HINTS_DECORATIONS;
			hints.functions = MwmHints::MWM_FUNC_MOVE | MwmHints::MWM_FUNC_CLOSE | MwmHints::MWM_FUNC_RESIZE;
			hints.decorations = MwmHints::MWM_DECOR_BORDER | MwmHints::MWM_DECOR_RESIZEH | MwmHints::MWM_DECOR_TITLE | MwmHints::MWM_DECOR_MENU;
			if (style.minimizeButton) {
				hints.functions |= MwmHints::MWM_FUNC_MINIMIZE;
				hints.decorations |= MwmHints::MWM_DECOR_MINIMIZE;
			}
			if (style.maximizeButton) {
				hints.functions |= MwmHints::MWM_FUNC_MAXIMIZE;
				hints.decorations |= MwmHints::MWM_DECOR_MAXIMIZE;
			}

			XChangeProperty(_linux.dis, _linux.window, _linux.MOTIF_WM_HINTS, XA_ATOM, 32, PropModeReplace, (uint8_t*)&hints, 5);
		}

		return true;
#endif
		return false;
	}

	void Application::toggleFullscreen() {
#if AE_OS == AE_OS_WIN
		if (_win.hWnd) {
			_isFullscreen = !_isFullscreen;

			bool visibled = isVisible();

			if (_isFullscreen) {
				GetClientRect(_win.hWnd, &_win.lastWndClientRect);
				_recordWindowRect();
			}

			_updateWindowRectValue();
			_changeWindow(true, true);
			if (visibled) ShowWindow(_win.hWnd, SW_SHOWDEFAULT);

			_eventDispatcher.dispatchEvent(this, ApplicationEvent::RESIZED);
		}
#endif
	}

	void Application::getClientSize(Vec2ui32& size) const {
#if AE_OS == AE_OS_WIN
		RECT rect;
		GetClientRect(_win.hWnd, &rect);
		size.set(rect.right - rect.left, rect.bottom - rect.top);
#else
		size.set(0);
#endif
	}

	void Application::getWindowRect(Box2i32ui32& dst) const {
#if AE_OS == AE_OS_WIN
		if (!_isFullscreen) _recordWindowRect();
#endif
		dst = _windowRect;
	}

	void Application::setWindowRect(const Box2i32ui32& clientRect) {
		Box2i32ui32 out;
		if (_adjustWindowRect(clientRect, out)) {
#if AE_OS == AE_OS_WIN
			_win.lastWndClientRect.left = clientRect.pos[0];
			_win.lastWndClientRect.right = clientRect.pos[0] + clientRect.size[0];
			_win.lastWndClientRect.top = clientRect.pos[1];
			_win.lastWndClientRect.bottom = clientRect.pos[1] + clientRect.size[1];

			if (_windowRect != out) {
				_windowRect = out;

				if (!_isFullscreen) {
					_updateWindowRectValue();
					_changeWindow(false, true);
				}
			}
#endif
		}
	}

	void Application::setWindowTitle(const std::string_view& title) {
#if AE_OS == AE_OS_WIN
		if (_win.hWnd) SetWindowTextW(_win.hWnd, String::Utf8ToUnicode(title).data());
#endif
	}

	void Application::setCursorVisible(bool visible) {
#if AE_OS == AE_OS_WIN
		ShowCursor(visible);
#endif
	}

	bool Application::hasFocus() const {
#if AE_OS == AE_OS_WIN
		return GetForegroundWindow() == _win.hWnd;
#endif
		return true;
	}

	void Application::pollEvents() {
#if AE_OS == AE_OS_WIN
		MSG msg;
		memset(&msg, 0, sizeof(msg));

		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				_isClosing = true;
			} else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
#elif AE_OS == AE_OS_LINUX
		XEvent e = { 0 };

		while (XCheckIfEvent(_linux.dis, &e, [](Display*, XEvent*, XPointer) {
			return 1;
		}, nullptr)) {}
#endif
	}

	bool Application::isVisible() const {
#if AE_OS == AE_OS_WIN
		return _win.hWnd ? IsWindowVisible(_win.hWnd) : false;
#elif AE_OS == AE_OS_LINUX
		return _linux.isVisible;
#endif
		return false;
	}

	void Application::setVisible(bool b) {
#if AE_OS == AE_OS_WIN
		if (_win.hWnd) {
			if (b) {
				ShowWindow(_win.hWnd, SW_SHOWDEFAULT);
			} else {
				ShowWindow(_win.hWnd, SW_HIDE);
			}

			UpdateWindow(_win.hWnd);
		}
#elif AE_OS == AE_OS_LINUX
		if (_linux.dis) {
			if (_linux.isVisible != b) {
				_linux.isVisible = b;
				if (b) {
					XMapWindow(_linux.dis, _linux.window);
				} else {
					XUnmapWindow(_linux.dis, _linux.window);
				}
			}
		}
#endif
	}

	void Application::shutdown() {
#if AE_OS == AE_OS_WIN
		PostQuitMessage(0);
#endif
	}

	bool Application::_adjustWindowRect(const Box2i32ui32& in, Box2i32ui32& out) {
#if AE_OS == AE_OS_WIN
		RECT rect = { in.pos[0], in.pos[1], in.pos[0] + in.size[0], in.pos[1] + in.size[1] };
		auto rst = AdjustWindowRectEx(&rect, _getWindowStyle(), FALSE, _getWindowExStyle());
		out.set(Vec2i32(rect.left, rect.top), Vec2i32(rect.right - rect.left, rect.bottom - rect.top));
		return rst;
#else
		out.set(in);
		return true;
#endif
	}

	void Application::_recordWindowRect() const {
#if AE_OS == AE_OS_WIN
		RECT rect;
		GetWindowRect(_win.hWnd, &rect);
		_windowRect.set(Vec2i32(rect.left, rect.top), Vec2i32(rect.right - rect.left, rect.bottom - rect.top));
#endif
	}

#if AE_OS == AE_OS_WIN
	DWORD Application::_getWindowStyle() const {
		DWORD style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

		if (_isFullscreen) {
			style |= WS_POPUP;
		} else {
			style |= WS_CAPTION | WS_SYSMENU;
			if (_style.minimizeButton) style |= WS_MINIMIZEBOX;
			if (_style.maximizeButton) style |= WS_MAXIMIZEBOX;
			if (_style.thickFrame) style |= WS_THICKFRAME;
		}

		return style;
	}

	DWORD Application::_getWindowExStyle() const {
		DWORD style = WS_EX_APPWINDOW;

		if (_isFullscreen) {
			//style |= WS_EX_TOPMOST;
		}

		return style;
	}

	void Application::_updateWindowRectValue() {
		if (_isFullscreen) {
			_wndRect.set(Vec2i32::ZERO, Vec2i32(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)));
		} else {
			_wndRect = _windowRect;
		}
	}

	void Application::_changeWindow(bool style, bool posOrSize) {
		if (style) {
			SetWindowLongPtr(_win.hWnd, GWL_STYLE, _getWindowStyle());
			SetWindowLongPtr(_win.hWnd, GWL_EXSTYLE, _getWindowExStyle());
		}
		if (posOrSize) {
			uint32_t flags = SWP_NOACTIVATE;
			if (_isFullscreen) {
				flags |= SWP_NOCOPYBITS;
			} else {
				flags |= SWP_NOOWNERZORDER | SWP_NOZORDER;
			}
			SetWindowPos(_win.hWnd, HWND_NOTOPMOST, _wndRect.pos[0], _wndRect.pos[1], _wndRect.size[0], _wndRect.size[1], flags);
		}
	}

	LRESULT Application::_wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		auto app = (Application*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

		switch (msg) {
		case WM_CLOSE:
		{
			if (app) {
				bool isCanceled = false;
				app->_eventDispatcher.dispatchEvent(app, ApplicationEvent::CLOSING, &isCanceled);
				if (isCanceled) {
					return 0;
				} else {
					app->_eventDispatcher.dispatchEvent(app, ApplicationEvent::CLOSED);
				}
			}

			break;
		}
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		case WM_SIZE:
			if (app) app->_eventDispatcher.dispatchEvent(app, ApplicationEvent::RESIZED);
			break;
		case WM_SETFOCUS:
			if (app) app->_eventDispatcher.dispatchEvent(app, ApplicationEvent::FOCUS_IN);
			break;
		case WM_KILLFOCUS:
			if (app) app->_eventDispatcher.dispatchEvent(app, ApplicationEvent::FOCUS_OUT);
			break;
		case WM_DEVICECHANGE:
			break;
		case WM_GETMINMAXINFO:
		{
			/*
			auto info = (tagMINMAXINFO*)lParam;
			info->ptMinTrackSize.x = 10;
			info->ptMinTrackSize.y = 10;
			info->ptMaxTrackSize.x = 400;
			info->ptMaxTrackSize.y = 400;
			*/

			break;
		}
		default:
			break;
		}

		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
#endif
}