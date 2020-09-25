#include "Application.h"
#include "aurora/String.h"
#include "aurora/Debug.h"

namespace aurora {
	Application::Application(const std::string_view& appId) :
		_isFullscreen(false),
		_isClosing(false),
		_isVisible(false),
		_appId(appId) {
		_appPath = aurora::getAppPath();
#if AE_OS == AE_OS_WIN
		_win.sentSize.set((std::numeric_limits<decltype(_win.sentSize)::ElementType>::max)());
#elif AE_OS == AE_OS_LINUX
		_linux.sentSize.set((std::numeric_limits<decltype(_linux.sentSize)::ElementType>::max)());
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

		auto rect = _calcWindowRect(false);
		{
			_win.clinetPos.set((GetSystemMetrics(SM_CXSCREEN) - rect.size[0]) / 2 + _border[0], (GetSystemMetrics(SM_CYSCREEN) - rect.size[1]) / 2 + _border[2]);

			auto area = _calcWorkArea();
			_win.clinetPos.set(area.pos[0] + (area.size[0] - rect.size[0]) / 2 + _border[0], area.pos[1] + (area.size[1] - rect.size[1]) / 2 + _border[2]);

			if (_win.clinetPos[1] < GetSystemMetrics(SM_CYCAPTION)) _win.clinetPos[1] = GetSystemMetrics(SM_CYCAPTION);
		}

		if (_isFullscreen) {
			rect = _calcWindowRect(true);
		} else {
			rect.pos.set(_win.clinetPos[0], _win.clinetPos[1]);
		}

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
		_linux.WM_STATE = XInternAtom(_linux.dis, "WM_STATE", False);
		_linux.WM_PROTOCOLS = XInternAtom(_linux.dis, "WM_PROTOCOLS", False);
		_linux.WM_DELETE_WINDOW = XInternAtom(_linux.dis, "WM_DELETE_WINDOW", False);
		_linux.NET_WM_PING = XInternAtom(_linux.dis, "_NET_WM_PING", False);
		_linux.NET_WM_WINDOW_TYPE = XInternAtom(_linux.dis, "_NET_WM_WINDOW_TYPE", False);
		_linux.NET_WM_WINDOW_TYPE_NORMAL = XInternAtom(_linux.dis, "_NET_WM_WINDOW_TYPE_NORMAL", False);
		_linux.NET_WM_STATE = XInternAtom(_linux.dis, "_NET_WM_STATE", False);
		_linux.NET_WM_STATE_FULLSCREEN = XInternAtom(_linux.dis, "_NET_WM_STATE_FULLSCREEN", False);
		_linux.NET_WM_STATE_MAXIMIZED_HORZ = XInternAtom(_linux.dis, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
		_linux.NET_WM_STATE_MAXIMIZED_VERT = XInternAtom(_linux.dis, "_NET_WM_STATE_MAXIMIZED_VERT", False);
		_linux.NET_REQUEST_FRAME_EXTENTS = XInternAtom(_linux.dis, "_NET_REQUEST_FRAME_EXTENTS", False);
		_linux.NET_FRAME_EXTENTS = XInternAtom(_linux.dis, "_NET_FRAME_EXTENTS", False);
		_linux.NET_WORKAREA = XInternAtom(_linux.dis, "_NET_WORKAREA", False);
		_linux.NET_CURRENT_DESKTOP = XInternAtom(_linux.dis, "_NET_CURRENT_DESKTOP", False);

		_linux.bgColor = _style.backgroundColor[0] < 16 | _style.backgroundColor[1] < 8 | _style.backgroundColor[2];

		XSetWindowAttributes attr = { 0 };
		attr.border_pixel = 0;
		attr.background_pixel = _linux.bgColor;
		attr.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask |
			PointerMotionMask | ButtonPressMask | ButtonReleaseMask |
			ExposureMask | FocusChangeMask | VisibilityChangeMask |
			EnterWindowMask | LeaveWindowMask | PropertyChangeMask;

		_linux.screen = DefaultScreen(_linux.dis);
		_linux.root = RootWindow(_linux.dis, _linux.screen);
		_linux.wnd = XCreateWindow(_linux.dis, _linux.root,
			0, 0, _clientSize[0], _clientSize[1], 0,
			DefaultDepth(_linux.dis, 0), InputOutput, DefaultVisual(_linux.dis, 0), CWBorderPixel | CWColormap | CWEventMask, &attr);

		//XSelectInput(_linux.dis, _linux.wnd, ExposureMask | ButtonPressMask | KeyPressMask | PropertyChangeMask);

		//auto sd = ScreenOfDisplay(_linux.dis, _linux.screen);
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

			_sendClientEventToWM(_linux.NET_REQUEST_FRAME_EXTENTS);
			_waitEvent(_linux.waitFrameEXTENTS);
		}

		{
			auto area = _calcWorkArea();
			//auto sd = ScreenOfDisplay(_linux.dis, _linux.screen);
			_linux.wndPos.set((area.size[0] - _clientSize[0] - _border[0] - _border[1]) / 2, (area.size[1] - _clientSize[1] - _border[2] - _border[3]) / 2);

			if (auto top = area.pos[1] + _border[2]; _linux.wndPos[1] < top) _linux.wndPos[1] = top;
		}

		_updateWindowPlacement();

		printcln("state :   ", _getXWndState(), "   ", NormalState, "   ", IconicState);

		return true;
#endif
		return false;
	}

	void* Application::getNativeWindow() const {
#if AE_OS == AE_OS_WIN
		return _win.wnd;
#elif AE_OS == AE_OS_LINUX
		return (void*)_linux.wnd;
#else
		return nullptr;
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

			if (_isVisible) {
				_win.ignoreEvtSize = true;
				_updateWindowPlacement();
				_win.ignoreEvtSize = false;
			}
			_sendResizedEvent();
		}
#elif AE_OS == AE_OS_LINUX
		if (_linux.wnd) {
			_isFullscreen = !_isFullscreen;
			_updateWindowPlacement();
		}
#endif
	}

	Vec2ui32 Application::getCurrentClientSize() const {
		Vec2ui32 size;
#if AE_OS == AE_OS_WIN
		if (_win.wnd) {
			if (_isVisible && _win.wndState != WindowState::MINIMUM) {
				RECT rect;
				GetClientRect(_win.wnd, &rect);
				size.set(rect.right - rect.left, rect.bottom - rect.top);
			} else {
				if (_isFullscreen) {
					size.set(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
				} else {
					if (_win.wndState == WindowState::MAXIMUM || (_win.wndState == WindowState::MINIMUM && _win.prevWndState == WindowState::MAXIMUM)) {
						auto area = _calcWorkArea();
						size.set(std::max<int32_t>(area.size[0], 0), std::max<int32_t>(area.size[1] - GetSystemMetrics(SM_CYCAPTION), 0));
					} else {
						size = _clientSize;
					}
				}
			}
		}
#elif AE_OS == AE_OS_LINUX
		if (_linux.wnd) {
			if (_isFullscreen) {
				auto sd = ScreenOfDisplay(_linux.dis, _linux.screen);
				size.set(sd->width, sd->height);
			} else {
				if (_linux.wndState == WindowState::MAXIMUM || (_linux.wndState == WindowState::MINIMUM && _linux.prevWndState == WindowState::MAXIMUM)) {
					auto area = _calcWorkArea();
					size.set(std::max<int32_t>(area.size[0], 0), std::max<int32_t>(area.size[1] - _border[2], 0));
				} else {
					size = _clientSize;
				}
			}
		}
#endif
		return size;
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
		if (_linux.wnd && _clientSize != size) {
			_clientSize = size;
			_updateWindowPlacement();
			_sendResizedEvent();
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
			if (auto p = pos + _border.components(0, 2); _win.clinetPos != p) {
				_win.clinetPos = p;
				_win.wndDirty = true;

				if (!_isFullscreen && _isVisible && _win.wndState == WindowState::NORMAL) _updateWindowPlacement(SW_SHOWNA);
			}
		}
#elif AE_OS == AE_OS_LINUX
		if (_linux.wnd && _linux.wndPos != pos) {
			_linux.wndPos = pos;
			_updateWindowPlacement();
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
		if (_win.wnd) return GetFocus() == _win.wnd;
#elif AE_OS == AE_OS_LINUX
		if (_linux.wnd) {
			Window focused;
			int32_t revertTo;

			XGetInputFocus(_linux.dis, &focused, &revertTo);
			return focused == _linux.wnd;
		}
#endif
		return false;
	}

	void Application::setFocus() {
#if AE_OS == AE_OS_WIN
		if (_win.wnd) SetFocus(_win.wnd);
#elif AE_OS == AE_OS_LINUX
		if (_linux.wnd && _isVisible) XSetInputFocus(_linux.dis, _linux.wnd, RevertToParent, CurrentTime);
#endif
	}

	bool Application::isMaximzed() const {
#if AE_OS == AE_OS_WIN
		if (_win.wnd) return _win.wndState == WindowState::MAXIMUM;
#elif AE_OS == AE_OS_LINUX
		if (_linux.wnd) return _linux.wndState == WindowState::MAXIMUM;
#endif
		return false;
	}

	void Application::setMaximum() {
#if AE_OS == AE_OS_WIN
		if (_win.wnd && _setWndState(WindowState::MAXIMUM)) {
			_win.wndDirty = true;
			if (!_isFullscreen && _isVisible) _updateWindowPlacement();
			_sendResizedEvent();
		}
#elif AE_OS == AE_OS_LINUX
		if (_linux.wnd && _setWndState(WindowState::MAXIMUM)) _updateWindowPlacement();
#endif
	}

	bool Application::isMinimzed() const {
#if AE_OS == AE_OS_WIN
		if (_win.wnd) return _win.wndState == WindowState::MINIMUM;
#elif AE_OS == AE_OS_LINUX
		if (_linux.wnd) return _linux.wndState == WindowState::MINIMUM;
#endif
		return false;
	}

	void Application::setMinimum() {
#if AE_OS == AE_OS_WIN
		if (_win.wnd && _setWndState(WindowState::MINIMUM)) {
			_win.wndDirty = true;
			if (!_isFullscreen && _isVisible) _updateWindowPlacement();
			_sendResizedEvent();
		}
#elif AE_OS == AE_OS_LINUX
		if (_linux.wnd && _setWndState(WindowState::MINIMUM)) _updateWindowPlacement();
#endif
	}

	void Application::setRestore() {
#if AE_OS == AE_OS_WIN
		if (_win.wnd && _setWndState(WindowState::NORMAL)) {
			_win.wndDirty = true;
			if (!_isFullscreen && _isVisible) _updateWindowPlacement();
			_sendResizedEvent();
		}
#elif AE_OS == AE_OS_LINUX
		if (_linux.wnd && _setWndState(WindowState::NORMAL)) _updateWindowPlacement();
#endif
	}

	void Application::pollEvents() {
#if AE_OS == AE_OS_WIN
		MSG msg = { 0 };

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
		while (XCheckIfEvent(_linux.dis, &e, &Application::_eventPredicate, (XPointer)this)) _doEvent(e);
#endif
	}

	bool Application::isVisible() const {
#if AE_OS == AE_OS_WIN
		return _win.wnd ? _isVisible : false;
#elif AE_OS == AE_OS_LINUX
		return _linux.wnd ? _isVisible : false;
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
		if (_linux.wnd && _isVisible != b) {
			_isVisible = b;
			_updateWindowPlacement();
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

	Box2i32 Application::_calcWindowRect(bool fullscreen) const {
		if (fullscreen) {
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

	Box2i32 Application::_calcWorkArea() const {
		Box2i32 area;

		tagPOINT p{ 0 };
		if (auto monitor = MonitorFromPoint(p, MONITOR_DEFAULTTONEAREST); monitor) {
			MONITORINFO info{ 0 };
			info.cbSize = sizeof(info);
			if (GetMonitorInfo(monitor, &info)) {
				auto& rcWork = info.rcWork;
				area.pos.set(rcWork.left, rcWork.top);
				area.size.set(rcWork.right - rcWork.left, rcWork.bottom - rcWork.top);
			}
		}

		return std::move(area);
	}

	bool Application::_setWndState(WindowState state) {
		if (_win.wndState != state) {
			_win.prevWndState = _win.wndState;
			_win.wndState = state;
			return true;
		}

		return false;
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

			if (showCmd != (std::numeric_limits<decltype(showCmd)>::max)()) info.showCmd = showCmd;

			auto rect = _calcWindowRect(_isFullscreen);
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
		DWORD val = WS_EX_APPWINDOW;

		//if (fullscreen) val |= WS_EX_TOPMOST;

		return val;
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
				if (app->_win.ignoreEvtSize) return 0;

				if (!app->_isFullscreen) {
					switch (wParam) {
					case SIZE_MINIMIZED:
						app->_setWndState(WindowState::MINIMUM);
						break;
					case SIZE_MAXIMIZED:
						app->_setWndState(WindowState::MAXIMUM);
						break;
					case SIZE_RESTORED:
						app->_setWndState(WindowState::NORMAL);
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
	void Application::_sendClientEventToWM(Atom msgType, int64_t a, int64_t b, int64_t c, int64_t d, int64_t e) {
		XEvent evt = { ClientMessage };
		auto& client = evt.xclient;
		client.window = _linux.wnd;
		client.format = 32; // Data is 32-bit longs
		client.message_type = msgType;
		client.data.l[0] = a;
		client.data.l[1] = b;
		client.data.l[2] = c;
		client.data.l[3] = d;
		client.data.l[4] = e;
		XSendEvent(_linux.dis, _linux.root, False, SubstructureNotifyMask | SubstructureRedirectMask, &evt);
	}

	void Application::_sendResizedEvent() {
		if (auto size = getCurrentClientSize(); _linux.sentSize != size) {
			_linux.sentSize = size;
			_eventDispatcher.dispatchEvent(this, ApplicationEvent::RESIZED);
		}
	}

	Bool Application::_eventPredicate(Display* display, XEvent* event, XPointer pointer) {
		return True;
	}

	void Application::_waitEvent(bool& value) {
		while (value) {
			XEvent e = { 0 };
			if (XCheckIfEvent(_linux.dis, &e, &Application::_eventPredicate, (XPointer)this)) _doEvent(e);
		};
	}

	size_t Application::_getXWndProperty(Window wnd, Atom property, Atom type, uint8_t** value) const {
		Atom actualType;
		int32_t actualFormat;
		uint64_t count, bytesAfter;

		XGetWindowProperty(_linux.dis, wnd, property, 0, (~0), False, type, &actualType, &actualFormat, &count, &bytesAfter, value);

		return count;
	}

	bool Application::_isMaximized() const {
		Atom* states = nullptr;
		bool maximized = false;

		auto count = _getXWndProperty(_linux.wnd, _linux.NET_WM_STATE, XA_ATOM, (uint8_t**)&states);

		for (size_t i = 0; i < count; i++) {
			if (states[i] == _linux.NET_WM_STATE_MAXIMIZED_VERT || states[i] == _linux.NET_WM_STATE_MAXIMIZED_HORZ) {
				maximized = true;
				break;
			}
		}

		if (states) XFree(states);

		return maximized;
	}

	bool Application::_isMinimized() const {
		return _getXWndState() == IconicState;
	}

	int32_t Application::_getXWndState() const {
		int32_t result = WithdrawnState;
		struct {
			uint32_t state;
			Window icon;
		}*state = nullptr;

		if (_getXWndProperty(_linux.wnd, _linux.WM_STATE, _linux.WM_STATE, (uint8_t**)&state) >= 2) result = state->state;

		if (state) XFree(state);

		return result;
	}

	Box2i32 Application::_calcWorkArea() const {
		Box2i32 area;

		Atom* extents = nullptr;
		Atom* desktop = nullptr;

		if (auto extentCount = _getXWndProperty(_linux.root, _linux.NET_WORKAREA, XA_CARDINAL, (uint8_t**)&extents); extentCount >= 4) {
			if (_getXWndProperty(_linux.root, _linux.NET_CURRENT_DESKTOP, XA_CARDINAL, (uint8_t**)&desktop) > 0) {
				if (*desktop < extentCount / 4) {
					area.pos.set(extents[*desktop * 4 + 0], extents[*desktop * 4 + 1]);
					area.size.set(extents[*desktop * 4 + 2], extents[*desktop * 4 + 3]);
				}
			}
		}

		if (extents) XFree(extents);
		if (desktop) XFree(desktop);

		return std::move(area);
	}

	bool Application::_setWndState(WindowState state) {
		if (_linux.wndState != state) {
			_linux.prevWndState = _linux.wndState;
			_linux.wndState = state;
			return true;
		}

		return false;
	}

	void Application::_updateWindowPlacement() {
		if (_isVisible) {
			if (!_linux.xMapped) {
				_linux.xMapped = true;
				_linux.waitVisibility = true;
				XMapWindow(_linux.dis, _linux.wnd);
			}

			if (_isFullscreen) {
				if (!_linux.xFullscreen) {
					_linux.xFullscreen = true;
					_sendClientEventToWM(_linux.NET_WM_STATE, 1, _linux.NET_WM_STATE_FULLSCREEN, 0, 1);
				}
			} else {
				if (_linux.xFullscreen) {
					_linux.xFullscreen = false;
					_sendClientEventToWM(_linux.NET_WM_STATE, 0, _linux.NET_WM_STATE_FULLSCREEN, 0, 1);
				}

				switch (_linux.wndState) {
				case WindowState::MAXIMUM:
				{
					if (_linux.xWndState != WindowState::MAXIMUM) {
						_linux.xWndState = WindowState::MAXIMUM;
						_sendClientEventToWM(_linux.NET_WM_STATE, 1, _linux.NET_WM_STATE_MAXIMIZED_VERT, _linux.NET_WM_STATE_MAXIMIZED_HORZ, 1);
					}

					break;
				}
				case WindowState::MINIMUM:
				{
					if (_linux.xWndState != WindowState::MINIMUM) {
						if (_linux.xWndState == WindowState::MAXIMUM) {
							_sendClientEventToWM(_linux.NET_WM_STATE, 0, _linux.NET_WM_STATE_MAXIMIZED_VERT, _linux.NET_WM_STATE_MAXIMIZED_HORZ, 1);
						}

						_linux.xWndState = WindowState::MINIMUM;
						XIconifyWindow(_linux.dis, _linux.wnd, _linux.screen);
					}

					break;
				}
				default:
				{
					if (_linux.xWndState == WindowState::MINIMUM) {
						_linux.xWndState = WindowState::NORMAL;
						_linux.waitVisibility = true;
						XMapWindow(_linux.dis, _linux.wnd);
						_waitEvent(_linux.waitVisibility);
					} else if (_linux.xWndState == WindowState::MAXIMUM) {
						_sendClientEventToWM(_linux.NET_WM_STATE, 0, _linux.NET_WM_STATE_MAXIMIZED_VERT, _linux.NET_WM_STATE_MAXIMIZED_HORZ, 1);
					}

					XMoveWindow(_linux.dis, _linux.wnd, _linux.wndPos[0], _linux.wndPos[1]);

					break;
				}
				}
			}
		} else {
			if (_linux.xMapped) {
				_linux.xMapped = false;
				_linux.waitVisibility = true;
				XUnmapWindow(_linux.dis, _linux.wnd);
			}
		}

		_waitEvent(_linux.waitVisibility);
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
			auto isNormal = false;
			auto& conf = e.xconfigure;

			if (!_isFullscreen) {
				if (_isMaximized()) {
					_setWndState(WindowState::MAXIMUM);
				} else if (_isMinimized()) {
					_setWndState(WindowState::MINIMUM);
				} else {
					isNormal = true;
					_setWndState(WindowState::NORMAL);
				}
			}

			if (isNormal) {
				int32_t x, y;
				Window dummy;
				XTranslateCoordinates(_linux.dis, _linux.wnd, _linux.root, 0, 0, &x, &y, &dummy);

				_linux.wndPos.set(x - _border[0], y - _border[2]);
				_clientSize.set(conf.width, conf.height);
			}

			_sendResizedEvent();

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
			auto& atom = prop.atom;
			if (atom == _linux.NET_FRAME_EXTENTS) {
				if (prop.state == PropertyNewValue && prop.window == _linux.wnd) {
					uint8_t* property = nullptr;
					if (auto count = _getXWndProperty(_linux.wnd, _linux.NET_FRAME_EXTENTS, XA_CARDINAL, &property); count >= 4) {
						for (size_t i = 0; i < 4; ++i) _border[i] = ((int64_t*)property)[i];
					}

					if (property) XFree(property);

					_linux.waitFrameEXTENTS = false;
				}
			} else if (atom == _linux.WM_STATE) {
				if (!_isFullscreen) {
					if (auto state = _getXWndState(); state == IconicState) {
						_setWndState(WindowState::MINIMUM);
					} else if (state == NormalState) {
						_setWndState(WindowState::NORMAL);
					}
				}
			} else if (atom == _linux.NET_WM_STATE) {
				if (!_isFullscreen && _isMaximized()) _setWndState(WindowState::MAXIMUM);
			}

			break;
		}
		case VisibilityNotify:
		{
			_linux.waitVisibility = false;

			break;
		}
		default:
			break;
		}
	}
#endif
}