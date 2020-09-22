#include "Application.h"
#include "aurora/String.h"
#include "aurora/Debug.h"

namespace aurora {
	Application::Application(const std::string_view& appId) :
		_isFullscreen(false),
		_isClosing(false),
		_isVisible(false),
#if AE_OS == AE_OS_WIN
#elif AE_OS == AE_OS_LINUX
#endif
		_appId(appId) {
		_appPath = aurora::getAppPath();
#if AE_OS == AE_OS_WIN
		_win.sentSize.set((std::numeric_limits<decltype(_win.sentSize)::ElementType>::max)());
#endif
	}

	Application::~Application() {
#if AE_OS == AE_OS_WIN
		if (_win.wnd) {
			DestroyWindow(_win.wnd);
			_win.wnd = nullptr;
		}

		if (_win.bkBrush) {
			DeleteObject(_win.bkBrush);
			_win.bkBrush = nullptr;
		}

		if (_win.ins) {
			auto appIdW = String::Utf8ToUnicode(_appId);
			UnregisterClassW(appIdW.data(), _win.ins);
			_win.ins = nullptr;
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

	events::IEventDispatcher<ApplicationEvent>& Application::getEventDispatcher() {
		return _eventDispatcher;
	}

	const events::IEventDispatcher<ApplicationEvent>& Application::getEventDispatcher() const {
		return _eventDispatcher;
	}

	bool Application::createWindow(const ApplicationStyle& style, const std::string_view& title, const Vec2ui32& clientSize, bool fullscreen) {
		_clientSize = clientSize;
		_isFullscreen = fullscreen;
		_style = style;

#if AE_OS == AE_OS_WIN
		_win.ins = GetModuleHandle(nullptr);
		_win.bkBrush = CreateSolidBrush(RGB(style.backgroundColor[0], style.backgroundColor[1], style.backgroundColor[2]));

		WNDCLASSEXW wnd;
		memset(&wnd, 0, sizeof(wnd));
		wnd.cbSize = sizeof(wnd);
		wnd.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wnd.lpfnWndProc = Application::_wndProc;
		wnd.cbClsExtra = 0;
		wnd.cbWndExtra = 0;
		wnd.hInstance = _win.ins;
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
		{
			_win.clinetPos.set((GetSystemMetrics(SM_CXSCREEN) - rect.size[0]) / 2 + _border[0], (GetSystemMetrics(SM_CYSCREEN) - rect.size[1]) / 2 + _border[2]);

			tagPOINT p{ 0 };
			if (auto monitor = MonitorFromPoint(p, MONITOR_DEFAULTTONEAREST); monitor) {
				MONITORINFO info{ 0 };
				info.cbSize = sizeof(info);
				if (GetMonitorInfo(monitor, &info)) _win.clinetPos.set(info.rcWork.left + ((info.rcWork.right - info.rcWork.left) - rect.size[0]) / 2 + _border[0], info.rcWork.top + ((info.rcWork.bottom - info.rcWork.top) - rect.size[1]) / 2 + _border[2]);
			}

			if (_win.clinetPos[1] < GetSystemMetrics(SM_CYCAPTION)) _win.clinetPos[1] = GetSystemMetrics(SM_CYCAPTION);
		}

		if (!_isFullscreen) rect.pos.set(_win.clinetPos[0], _win.clinetPos[1]);
		_win.wnd = CreateWindowExW(_getWindowExStyle(_isFullscreen), wnd.lpszClassName, String::Utf8ToUnicode(title).data(), _getWindowStyle(_style, _isFullscreen),
			rect.pos[0], rect.pos[1], rect.size[0], rect.size[1],
			GetDesktopWindow(), nullptr, _win.ins, nullptr);
		SetWindowLongPtr(_win.wnd, GWLP_USERDATA, (LONG_PTR)this);

		//DeleteObject(bkBrush);

		return _win.wnd;
#elif AE_OS == AE_OS_LINUX
		if (!_linux.dis) _linux.dis = XOpenDisplay(nullptr);
		if (!_linux.dis) return false;

		_linux.MOTIF_WM_HINTS = XInternAtom(_linux.dis, "_MOTIF_WM_HINTS", False);
		_linux.WM_PROTOCOLS = XInternAtom(_linux.dis, "WM_PROTOCOLS", False);
		_linux.WM_DELETE_WINDOW = XInternAtom(_linux.dis, "WM_DELETE_WINDOW", False);
		_linux.NET_WM_PING = XInternAtom(_linux.dis, "_NET_WM_PING", False);
		_linux.NET_WM_WINDOW_TYPE = XInternAtom(_linux.dis, "_NET_WM_WINDOW_TYPE", False);
		_linux.NET_WM_WINDOW_TYPE_NORMAL = XInternAtom(_linux.dis, "_NET_WM_WINDOW_TYPE_NORMAL", False);
		_linux.NET_REQUEST_FRAME_EXTENTS = XInternAtom(_linux.dis, "_NET_REQUEST_FRAME_EXTENTS", False);
		_linux.NET_FRAME_EXTENTS = XInternAtom(_linux.dis, "_NET_FRAME_EXTENTS", False);

		_linux.bgColor = _style.backgroundColor[0] < 16 | _style.backgroundColor[1] < 8 | _style.backgroundColor[2];

		XSetWindowAttributes attr = { 0 };
		attr.border_pixel = 0;
		attr.background_pixel = _linux.bgColor;
		attr.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask |
			PointerMotionMask | ButtonPressMask | ButtonReleaseMask |
			ExposureMask | FocusChangeMask | VisibilityChangeMask |
			EnterWindowMask | LeaveWindowMask | PropertyChangeMask;

		_linux.root = RootWindow(_linux.dis, DefaultScreen(_linux.dis));
		_linux.wnd = XCreateWindow(_linux.dis, _linux.root,
			0, 0, _clientSize[0], _clientSize[1], 0,
			DefaultDepth(_linux.dis, 0), InputOutput, DefaultVisual(_linux.dis, 0), CWBorderPixel | CWColormap | CWEventMask, &attr);

		//XSelectInput(_linux.dis, _linux.wnd, ExposureMask | ButtonPressMask | KeyPressMask | PropertyChangeMask);

		//auto sd = ScreenOfDisplay(_linux.dis, DefaultScreen(_linux.dis));
		//printcln(sd->width, "   ", sd->height);

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
			Atom protocols[] = { _linux.WM_DELETE_WINDOW, _linux.NET_WM_PING };
			XSetWMProtocols(_linux.dis, _linux.wnd, protocols, sizeof(protocols) / sizeof(Atom));
		}

		{
			Atom type = _linux.NET_WM_WINDOW_TYPE_NORMAL;
			XChangeProperty(_linux.dis, _linux.wnd, _linux.NET_WM_WINDOW_TYPE, XA_ATOM, 32, PropModeReplace, (uint8_t*)&type, 1);
		}

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

		{
			auto hints = XAllocWMHints();
			hints->flags = StateHint;
			hints->initial_state = NormalState;

			XSetWMHints(_linux.dis, _linux.wnd, hints);
			XFree(hints);
		}

		setWindowTitle(title);

		XFlush(_linux.dis);

		{
			_linux.waitFrameEXTENTS = true;

			XEvent e = { ClientMessage };
			auto& c = e.xclient;
			c.window = _linux.wnd;
			c.format = 32; // Data is 32-bit longs
			c.message_type = _linux.NET_REQUEST_FRAME_EXTENTS;
			c.data.l[0] = 0;
			c.data.l[1] = 0;
			c.data.l[2] = 0;
			c.data.l[3] = 0;
			c.data.l[4] = 0;
			XSendEvent(_linux.dis, _linux.root, False, SubstructureNotifyMask | SubstructureRedirectMask, &e);

			_waitEvent(_linux.waitFrameEXTENTS);
		}

		{
			auto sd = ScreenOfDisplay(_linux.dis, DefaultScreen(_linux.dis));
			_linux.wndPos.set((sd->width - _clientSize[0] - _border[0] - _border[1]) / 2, (sd->height - _clientSize[1] - _border[2] - _border[3]) / 2);

			if (_linux.wndPos[1] < (decltype(_linux.wndPos)::ElementType)_border[2]) _linux.wndPos[1] = _border[2];
		}

		return true;
#endif
		return false;
	}

	uint64_t Application::getWindow() const {
#if AE_OS == AE_OS_WIN
		return (uint64_t)_win.wnd;
#elif AE_OS == AE_OS_LINUX
		return _linux.wnd;
#else
		return 0;
#endif
	}

	bool Application::isFullscreen() const {
		return _isFullscreen;
	}

	Vec4ui32 Application::getBorder() const {
		return _border;
	}

	void Application::toggleFullscreen() {
#if AE_OS == AE_OS_WIN
		if (_win.wnd) {
			_isFullscreen = !_isFullscreen;
			_win.wndDirty = true;

			SetWindowLongPtrW(_win.wnd, GWL_STYLE, _getWindowStyle(_style, _isFullscreen));
			SetWindowLongPtrW(_win.wnd, GWL_EXSTYLE, _getWindowExStyle(_isFullscreen));
			
			if (_isVisible) _updateWindowPlacement();
			_sendResizedEvent();
		}
#elif AE_OS == AE_OS_LINUX
		printcln("fs");
		XEvent e = { ClientMessage };
		auto& c = e.xclient;
		c.window = _linux.wnd;
		c.format = 32; // Data is 32-bit longs
		c.message_type = XInternAtom(_linux.dis, "_NET_WM_STATE", False);
		c.data.l[0] = 1;
		c.data.l[1] = XInternAtom(_linux.dis, "_NET_WM_STATE_FULLSCREEN", False);
		c.data.l[2] = 0;
		c.data.l[3] = 1;
		c.data.l[4] = 0;
		XSendEvent(_linux.dis, _linux.root, False, SubstructureNotifyMask | SubstructureRedirectMask, &e);
#endif
	}

	Vec2ui32 Application::getCurrentClientSize() const {
#if AE_OS == AE_OS_WIN
		Vec2ui32 size;
		if (_win.wnd) {
			if (_isVisible) {
				RECT rect;
				GetClientRect(_win.wnd, &rect);
				size.set(rect.right - rect.left, rect.bottom - rect.top);
			} else {
				if (_isFullscreen) {
					size.set(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
				} else {
					switch (_win.wndState) {
					case WindowState::MINIMUM:
						break;
					case WindowState::MAXIMUM:
					{
						tagPOINT p{ 0 };
						if (auto monitor = MonitorFromPoint(p, MONITOR_DEFAULTTONEAREST); monitor) {
							MONITORINFO info{ 0 };
							info.cbSize = sizeof(info);
							if (GetMonitorInfo(monitor, &info)) size.set(std::max<int32_t>(info.rcWork.right - info.rcWork.left, 0), std::max<int32_t>(info.rcWork.bottom - info.rcWork.top - GetSystemMetrics(SM_CYCAPTION), 0));
						}

						break;
					}
					default:
						size = _clientSize;
						break;
					}
				}
			}
		}

		return size;
#else
		return Vec2ui32();
#endif
	}

	Vec2ui32 Application::getClientSize() const {
		return _clientSize;
	}

	void Application::setClientSize(const Vec2ui32& size) {
#if AE_OS == AE_OS_WIN
		if (_win.wnd && _clientSize != size) {
			_clientSize = size;
			_win.wndDirty = true;

			if (!_isFullscreen && _isVisible && _win.wndState == WindowState::NORMAL) _updateWindowPlacement(SW_SHOWNA);
			_sendResizedEvent();
		}
#elif AE_OS == AE_OS_LINUX
		if (_linux.wnd) {
			if (_isFullscreen) {
				_clientSize = size;
			} else {
				setRestore();
				if (_clientSize != size) {
					_clientSize = size;
					XResizeWindow(_linux.dis, _linux.wnd, _clientSize[0], _clientSize[1]);
				}
			}
		}
#endif
	}

	void Application::setWindowTitle(const std::string_view& title) {
#if AE_OS == AE_OS_WIN
		if (_win.wnd) SetWindowTextW(_win.wnd, String::Utf8ToUnicode(title).data());
#elif AE_OS == AE_OS_LINUX
		if (_linux.wnd) XStoreName(_linux.dis, _linux.wnd, title.data());
#endif
	}

	void Application::setWindowPosition(const Vec2i32& pos) {
#if AE_OS == AE_OS_WIN
		if (_win.wnd) {
			_win.clinetPos.set(pos[0] + _border[0], pos[1] + _border[2]);
			_win.wndDirty = true;

			if (!_isFullscreen && _isVisible && _win.wndState == WindowState::NORMAL) _updateWindowPlacement(SW_SHOWNA);
		}
#elif AE_OS == AE_OS_LINUX
		if (_linux.wnd) {
			if (_isFullscreen) {
				_linux.wndPos = pos;
			} else {
				setRestore();
				_linux.wndPos = pos;
				if (_linux.isVisible) XMoveWindow(_linux.dis, _linux.wnd, _linux.wndPos[0], _linux.wndPos[1]);
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
		return GetFocus() == _win.wnd;
#elif AE_OS == AE_OS_LINUX
		if (_linux.wnd) {
			Window focused;
			int revertTo;

			XGetInputFocus(_linux.dis, &focused, &revertTo);
			return focused == _linux.wnd;
		}
#endif
		return false;
	}

	void Application::setFocus() {
#if AE_OS == AE_OS_WIN
		if (_win.wnd) SetFocus(_win.wnd);
#endif
	}

	bool Application::isMaximum() const {
#if AE_OS == AE_OS_WIN
		return _win.wnd ? _win.wndState == WindowState::MAXIMUM : false;
#endif
		return false;
	}

	void Application::setMaximum() {
#if AE_OS == AE_OS_WIN
		if (_win.wnd && _win.wndState != WindowState::MAXIMUM) {
			_win.wndState = WindowState::MAXIMUM;
			_win.wndDirty = true;
			if (!_isFullscreen && _isVisible) _updateWindowPlacement();
			_sendResizedEvent();
		}
#endif
	}

	bool Application::isMinimum() const {
#if AE_OS == AE_OS_WIN
		return _win.wnd ? _win.wndState == WindowState::MINIMUM : false;
#endif
		return false;
	}

	void Application::setMinimum() {
#if AE_OS == AE_OS_WIN
		if (_win.wnd && _win.wndState != WindowState::MINIMUM) {
			_win.wndState = WindowState::MINIMUM;
			_win.wndDirty = true;
			if (!_isFullscreen && _isVisible) _updateWindowPlacement();
			_sendResizedEvent();
		}
#endif
	}

	void Application::setRestore() {
#if AE_OS == AE_OS_WIN
		if (_win.wnd && _isFullscreen && _win.wndState != WindowState::NORMAL) {
			_win.wndState = WindowState::NORMAL;
			_win.wndDirty = true;
			if (!_isFullscreen && _isVisible) _updateWindowPlacement();
			_sendResizedEvent();
		}
#endif
	}

	void Application::pollEvents() {
#if AE_OS == AE_OS_WIN
		MSG msg;
		memset(&msg, 0, sizeof(msg));

		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				shutdown();
			} else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
#elif AE_OS == AE_OS_LINUX
		XEvent e = { 0 };
		while (XCheckIfEvent(_linux.dis, &e, &Application::_eventPredicate, (XPointer)this)) { _doEvent(e); }
#endif
	}

	bool Application::isVisible() const {
#if AE_OS == AE_OS_WIN
		return _win.wnd ? _isVisible : false;
#elif AE_OS == AE_OS_LINUX
		return _linux.wnd ? _linux.isVisible : false;
#endif
		return false;
	}

	void Application::setVisible(bool b) {
#if AE_OS == AE_OS_WIN
		if (_win.wnd && _isVisible != b) {
			_isVisible = b;
			_win.wndDirty = true;
			_updateWindowPlacement();
		}
#elif AE_OS == AE_OS_LINUX
		if (_linux.wnd) {
			if (_linux.isVisible != b) {
				_linux.isVisible = b;
				if (b) {
					XMapWindow(_linux.dis, _linux.wnd);
					XMoveWindow(_linux.dis, _linux.wnd, _linux.wndPos[0], _linux.wndPos[1]);
				} else {
					XUnmapWindow(_linux.dis, _linux.wnd);
				}
			}
		}
#endif
	}

	void Application::shutdown() {
		if (!_isClosing) {
			_isClosing = true;
			_eventDispatcher.dispatchEvent(this, ApplicationEvent::CLOSED);
			std::exit(0);
		}
	}

	std::string_view Application::getAppId() const {
		return _appId;
	}

	const std::filesystem::path& Application::getAppPath() const {
		return _appPath;
	}

#if AE_OS == AE_OS_WIN
	void Application::_calcBorder() {
		RECT rect = { 0, 0, 100, 100 };
		auto rst = AdjustWindowRectEx(&rect, _getWindowStyle(_style, false), FALSE, _getWindowExStyle(false));
		_border.set(-rect.left, rect.right - 100, -rect.top, rect.bottom - 100);
	}

	Box2i32 Application::_calcWindowRect() const {
		if (_isFullscreen) {
			return Box2i32(Vec2i32::ZERO, Vec2i32(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)));
		} else {
			return Box2i32(Vec2i32(_win.clinetPos[0] - _border[0], _win.clinetPos[1] - _border[2]), Vec2i32((Vec2i32::ElementType)_clientSize[0] + _border[0] + _border[1], (Vec2i32::ElementType)_clientSize[1] + _border[2] + _border[3]));
		}
	}

	void Application::_sendResizedEvent() {
		if (auto size = getCurrentClientSize(); _win.sentSize != size) {
			_win.sentSize = size;
			_eventDispatcher.dispatchEvent(this, ApplicationEvent::RESIZED);
		}
	}

	void Application::_updateWindowPlacement(UINT showCmd) {
		if (showCmd == (std::numeric_limits<UINT>::max)()) {
			if (_isVisible) {
				if (_isFullscreen) {
					showCmd = SW_SHOWDEFAULT;
				} else {
					switch (_win.wndState) {
					case WindowState::MAXIMUM:
						showCmd = SW_SHOWMAXIMIZED;
						break;
					case WindowState::MINIMUM:
						showCmd = SW_SHOWMINIMIZED;
						break;
					default:
						showCmd = SW_RESTORE;
						break;
					}
				}
			} else {
				showCmd = SW_HIDE;
			}
		}

		WINDOWPLACEMENT info = { sizeof(info) };
		GetWindowPlacement(_win.wnd, &info);

		if (_win.wndDirty || info.showCmd != showCmd) {
			_win.wndDirty = false;

			if (showCmd != (std::numeric_limits<UINT>::max)()) info.showCmd = showCmd;

			auto rect = _calcWindowRect();
			info.rcNormalPosition.left = rect.pos[0];
			info.rcNormalPosition.top = rect.pos[1];
			info.rcNormalPosition.right = rect.pos[0] + rect.size[0];
			info.rcNormalPosition.bottom = rect.pos[1] + rect.size[1];

			SetWindowPlacement(_win.wnd, &info);
		}
	}

	DWORD Application::_getWindowStyle(const ApplicationStyle& style, bool fullscreen) {
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
					app->shutdown();
				}
			}

			break;
		}
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		case WM_SIZING:
			break;
		case WM_WINDOWPOSCHANGED:
			break;
		case WM_SIZE:
		{
			if (app) {
				if (!app->_isFullscreen) {
					switch (wParam) {
					case SIZE_MINIMIZED:
						app->_win.wndState = WindowState::MINIMUM;
						break;
					case SIZE_MAXIMIZED:
						app->_win.wndState = WindowState::MAXIMUM;
						break;
					case SIZE_RESTORED:
						app->_win.wndState = WindowState::NORMAL;
						break;
					default:
						break;
					}

					if (wParam == SIZE_RESTORED) app->_clientSize.set(LOWORD(lParam), HIWORD(lParam));
				}

				app->_sendResizedEvent();
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
			if (app && !app->_isFullscreen && !IsZoomed(app->_win.wnd) && !IsIconic(app->_win.wnd)) app->_win.clinetPos.set(LOWORD(lParam), HIWORD(lParam));
			break;
		default:
			break;
		}

		return DefWindowProcW(hWnd, msg, wParam, lParam);
	}
#elif AE_OS == AE_OS_LINUX
	Bool Application::_eventPredicate(Display* display, XEvent* event, XPointer pointer) {
		return True;
	}

	void Application::_waitEvent(bool& value) {
		do {
			XEvent e = { 0 };
			if (XCheckIfEvent(_linux.dis, &e, &Application::_eventPredicate, (XPointer)this)) _doEvent(e);
		} while (value);
	}

	void Application::_doEvent(XEvent& e) {
		switch (e.type) {
		case ClientMessage:
		{
			auto msgType = e.xclient.message_type;

			if (msgType == _linux.WM_PROTOCOLS) {
				auto protocol = (Atom)e.xclient.data.l[0];

				if (protocol == _linux.WM_DELETE_WINDOW) {
					auto isCanceled = false;
					_eventDispatcher.dispatchEvent(this, ApplicationEvent::CLOSING, &isCanceled);
					if (!isCanceled) shutdown();
				} else if (protocol == _linux.NET_WM_PING) {
					XEvent reply = e;
					reply.xclient.window = _linux.root;

					XSendEvent(_linux.dis, _linux.root, False, SubstructureNotifyMask | SubstructureRedirectMask, &reply);
				}
			}

			break;
		}
		case ConfigureNotify:
		{
			auto& conf = e.xconfigure;

			int x, y;
			Window dummy;
			XTranslateCoordinates(_linux.dis, _linux.wnd, _linux.root, 0, 0, &x, &y, &dummy);

			_linux.wndPos.set(x - _border[0], y - _border[2]);
			printcln(_linux.wndPos[0], "   ", _linux.wndPos[1], "   ", conf.width, "   ", conf.height);
			if (_clientSize[0] != conf.width || _clientSize[1] != conf.height) {
				_clientSize.set(conf.width, conf.height);
				//XSetWindowBackground(_linux.dis, _linux.wnd, _linux.bgColor);

				_eventDispatcher.dispatchEvent(this, ApplicationEvent::RESIZED);
			}

			break;
		}
		case Expose:
		{
			//XFlush(_linux.dis);
			XSetWindowBackground(_linux.dis, _linux.wnd, _linux.bgColor);

			break;
		}
		case FocusIn:
		{
			auto mode = e.xfocus.mode;
			if (mode == NotifyGrab || mode == NotifyUngrab) return;

			_eventDispatcher.dispatchEvent(this, ApplicationEvent::FOCUS_IN);

			break;
		}
		case FocusOut:
		{
			auto mode = e.xfocus.mode;
			if (mode == NotifyGrab || mode == NotifyUngrab) return;

			_eventDispatcher.dispatchEvent(this, ApplicationEvent::FOCUS_OUT);

			break;
		}
		case PropertyNotify:
		{
			auto& prop = e.xproperty;
			if (prop.atom == _linux.NET_FRAME_EXTENTS && prop.state == PropertyNewValue && prop.window == _linux.wnd) {
				Atom type;
				int format;
				unsigned long nitems, bytes_after;
				unsigned char* property = nullptr;

				XGetWindowProperty(_linux.dis, _linux.wnd, _linux.NET_FRAME_EXTENTS, 0, 16, False, XA_CARDINAL, &type, &format, &nitems, &bytes_after, &property);
				if (format) {
					for (size_t i = 0; i < 4; ++i) _border[i] = ((long*)property)[i];
				}

				if (property) XFree(property);

				_linux.waitFrameEXTENTS = false;
			}

			break;
		}
		default:
			break;
		}
	}
#endif
}