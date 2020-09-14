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
		if (_linux.wnd) {
			XDestroyWindow(_linux.dis, _linux.wnd);
			_linux.wnd = 0;
		}

		if (_linux.dis) {
			XCloseDisplay(_linux.dis);
			_linux.dis = nullptr;
		}
#endif
	}

	bool Application::createWindow(const Style& style, const std::string_view& title, const Vec2ui32& clientSize, bool fullscreen) {
		_clientSize = clientSize;
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
		wnd.lpszMenuName = nullptr;;
		auto appIdW = String::Utf8ToUnicode(_appId);
		wnd.lpszClassName = appIdW.data();
		wnd.hIconSm = nullptr;

		RegisterClassExW(&wnd);

		_calcBorder();

		auto rect = _calcWindowRect();
		if (!_isFullscreen) rect.pos.set(_border[0], _border[1]);
		_win.hWnd = CreateWindowExW(_getWindowExStyle(_isFullscreen), wnd.lpszClassName, String::Utf8ToUnicode(title).data(), _getWindowStyle(_style, _isFullscreen),
			rect.pos[0], rect.pos[1], rect.size[0], rect.size[1],
			GetDesktopWindow(), nullptr, _win.hIns, nullptr);
		SetWindowLongPtr(_win.hWnd, GWLP_USERDATA, (LONG_PTR)this);

		//DeleteObject(bkBrush);

		return _win.hWnd;
#elif AE_OS == AE_OS_LINUX
		if (!_linux.dis) _linux.dis = XOpenDisplay(nullptr);
		if (!_linux.dis) return false;

		auto s = DefaultScreen(_linux.dis);

		XSetWindowAttributes attr = { 0 };
		attr.border_pixel = 0;
		attr.background_pixel = style.backgroundColor[0] < 16 | style.backgroundColor[1] < 8 | style.backgroundColor[2];

		_linux.wnd = XCreateWindow(_linux.dis, RootWindow(_linux.dis, s),
			clientRect.pos[0], clientRect.pos[1], clientRect.size[0], clientRect.size[1], 0,
			DefaultDepth(_linux.dis, 0), InputOutput, DefaultVisual(_linux.dis, 0), CWBackPixel, &attr);

		_linux.MOTIF_WM_HINTS = XInternAtom(_linux.dis, "_MOTIF_WM_HINTS", False);

		{
			// auto hints = XAllocSizeHints();
			// hints->flags = PMinSize | PMaxSize;
			// hints->min_width = 10;
			// hints->min_height = 10;
			// hints->max_width = 100;
			// hints->max_height = 100;
			// XSetWMNormalHints(_linux.dis, _linux.wnd, hints);
			// XFree(hints);
		}

		// Atom window_type = XInternAtom(_linux.dis, "_NET_WM_WINDOW_TYPE", False);
		// int64_t value = XInternAtom(_linux.dis, "_NET_WM_WINDOW_TYPE_NORMAL", False);
		// XChangeProperty(_linux.dis, _linux.wnd, window_type, XA_ATOM, 32, PropModeReplace, (uint8_t*)&value, 1);

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

			XChangeProperty(_linux.dis, _linux.wnd, _linux.MOTIF_WM_HINTS, XA_ATOM, 32, PropModeReplace, (uint8_t*)&hints, 5);
		}

		setWindowTitle(title);

		return true;
#endif
		return false;
	}

	void Application::toggleFullscreen() {
#if AE_OS == AE_OS_WIN
		if (_win.hWnd) {
			_isFullscreen = !_isFullscreen;

			bool visibled = isVisible();

			SetWindowLongPtr(_win.hWnd, GWL_STYLE, _getWindowStyle(_style, _isFullscreen));
			SetWindowLongPtr(_win.hWnd, GWL_EXSTYLE, _getWindowExStyle(_isFullscreen));

			auto rect = _calcWindowRect();
			::SetWindowPos(_win.hWnd, HWND_NOTOPMOST, rect.pos[0], rect.pos[1], rect.size[0], rect.size[1], _getWindowPosFlags());

			if (visibled) ShowWindow(_win.hWnd, SW_SHOWDEFAULT);

			_eventDispatcher.dispatchEvent(this, ApplicationEvent::RESIZED);
		}
#endif
	}

	Vec2ui32 Application::getCurrentClientSize() const {
#if AE_OS == AE_OS_WIN
		RECT rect;
		GetClientRect(_win.hWnd, &rect);
		return Vec2ui32(rect.right - rect.left, rect.bottom - rect.top);
#else
		return Vec2ui32();
#endif
	}

	void Application::setClientSize(const Vec2ui32& size) {
		if (_clientSize != size) {
			_clientSize = size;
#if AE_OS == AE_OS_WIN
			if (!_isFullscreen) {
				auto rect = _calcWindowRect();
				::SetWindowPos(_win.hWnd, HWND_NOTOPMOST, rect.pos[0], rect.pos[1], rect.size[0], rect.size[1], _getWindowPosFlags<false>());
			}
#endif
		}
	}

	void Application::setWindowTitle(const std::string_view& title) {
#if AE_OS == AE_OS_WIN
		if (_win.hWnd) SetWindowTextW(_win.hWnd, String::Utf8ToUnicode(title).data());
#elif AE_OS == AE_OS_LINUX
		if (_linux.wnd) XStoreName(_linux.dis, _linux.wnd, title.data());
#endif
	}

	void Application::setWindowPosition(const Vec2i32& pos) {
#if AE_OS == AE_OS_WIN
		if (_win.hWnd) {
			_win.clinetPos.set(pos[0] + _border[0], pos[1] + _border[2]);
			if (!_isFullscreen) {
				auto rect = _calcWindowRect();
				::SetWindowPos(_win.hWnd, HWND_NOTOPMOST, rect.pos[0], rect.pos[1], rect.size[0], rect.size[1], _getWindowPosFlags<false>());
			}
		}
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
		}, nullptr)) {
		}
#endif
	}

	bool Application::isVisible() const {
#if AE_OS == AE_OS_WIN
		return _win.hWnd ? IsWindowVisible(_win.hWnd) : false;
#elif AE_OS == AE_OS_LINUX
		return _linux.wnd ? _linux.isVisible : false;
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
		if (_linux.wnd) {
			if (_linux.isVisible != b) {
				_linux.isVisible = b;
				if (b) {
					XMapWindow(_linux.dis, _linux.wnd);
				} else {
					XUnmapWindow(_linux.dis, _linux.wnd);
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

	void Application::_calcBorder() {
#if AE_OS == AE_OS_WIN
		RECT rect = { 0, 0, 100, 100 };
		auto rst = AdjustWindowRectEx(&rect, _getWindowStyle(_style, false), FALSE, _getWindowExStyle(false));
		_border.set(-rect.left, rect.right - 100, -rect.top, rect.bottom - 100);
#endif
	}

#if AE_OS == AE_OS_WIN
	Box2i32ui32 Application::_calcWindowRect() const {
		if (_isFullscreen) {
			return Box2i32ui32(Vec2i32::ZERO, Vec2i32(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)));
		} else {
			return Box2i32ui32(Vec2i32(_win.clinetPos[0] - _border[0], _win.clinetPos[1] - _border[2]), Vec2ui32(_clientSize[0] + _border[0] + _border[1], _clientSize[1] + _border[2] + _border[3]));
		}
	}

	DWORD Application::_getWindowStyle(const Style& style, bool fullscreen) {
		DWORD val = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

		if (fullscreen) {
			val |= WS_POPUP;
		} else {
			val |= WS_BORDER | WS_DLGFRAME | WS_SYSMENU;
			if (style.minimizeButton) val |= WS_MINIMIZEBOX;
			if (style.maximizeButton) val |= WS_MAXIMIZEBOX;
			if (style.thickFrame) val |= WS_THICKFRAME;
		}

		return val;
	}

	DWORD Application::_getWindowExStyle(bool fullscreen) {
		DWORD style = WS_EX_APPWINDOW;

		if (fullscreen) {
			//style |= WS_EX_TOPMOST;
		}

		return style;
	}

	LRESULT Application::_wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		auto app = (Application*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

		switch (msg) {
		case WM_CLOSE:
		{
			if (app) {
				auto isCanceled = false;
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
		{
			if (app) {
				if (!app->_isFullscreen) app->_clientSize.set(LOWORD(lParam), HIWORD(lParam));
				app->_eventDispatcher.dispatchEvent(app, ApplicationEvent::RESIZED);
			}

			break;
		}
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
		case WM_MOVE:
		{
			if (app) {
				if (!app->_isFullscreen) app->_win.clinetPos.set(LOWORD(lParam), HIWORD(lParam));
			}

			break;
		}
		default:
			break;
		}

		return DefWindowProcW(hWnd, msg, wParam, lParam);
	}
#endif
}