#include "ApplicationWindows.h"

#if SRK_OS == SRK_OS_WINDOWS
#include "srk/String.h"
#include "srk/Debug.h"

namespace srk {
	Application::Application(const std::string_view& appId) :
		_isFullscreen(false),
		_isClosing(false),
		_isVisible(false),
		_appId(appId),
		_eventDispatcher(new events::EventDispatcher<ApplicationEvent>()) {
		_appPath = srk::getAppPath();
		_win.sentSize.set((std::numeric_limits<decltype(_win.sentSize)::ElementType>::max)());
	}

	Application::~Application() {
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
	}

	IntrusivePtr<events::IEventDispatcher<ApplicationEvent>> Application::getEventDispatcher() {
		return _eventDispatcher;
	}

	//const events::IEventDispatcher<ApplicationEvent>& Application::getEventDispatcher() const {
	//	return _eventDispatcher;
	//}

	bool Application::createWindow(const ApplicationStyle& style, const std::string_view& title, const Vec2ui32& clientSize, bool fullscreen) {
		_clientSize = clientSize;
		_isFullscreen = fullscreen;
		_style = style;

		_win.ins = GetModuleHandleW(nullptr);
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
	}

	void* Application::getNative(ApplicationNative native) const {
		switch (native) {
		case ApplicationNative::INSTANCE:
			return _win.ins;
		case ApplicationNative::WINDOW:
			return _win.wnd;
		}

		return nullptr;
	}

	bool Application::isFullscreen() const {
		return _isFullscreen;
	}

	Vec4ui32 Application::getBorder() const {
		return _border;
	}

	void Application::toggleFullscreen() {
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
	}

	Vec2ui32 Application::getCurrentClientSize() const {
		Vec2ui32 size;

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

		return size;
	}

	Vec2ui32 Application::getClientSize() const {
		return _clientSize;
	}

	void Application::setClientSize(const Vec2ui32& size) {
		if (_win.wnd && _clientSize != size) {
			_clientSize = size;
			_win.wndDirty = true;

			if (!_isFullscreen && _isVisible && _win.wndState == WindowState::NORMAL) _updateWindowPlacement(SW_SHOWNA);
			_sendResizedEvent();
		}
	}

	void Application::setWindowTitle(const std::string_view& title) {
		if (_win.wnd) SetWindowTextW(_win.wnd, String::Utf8ToUnicode(title).data());
	}

	void Application::setWindowPosition(const Vec2i32& pos) {
		if (_win.wnd) {
			if (auto p = pos + _border.components(0, 2); _win.clinetPos != p) {
				_win.clinetPos = p;
				_win.wndDirty = true;

				if (!_isFullscreen && _isVisible && _win.wndState == WindowState::NORMAL) _updateWindowPlacement(SW_SHOWNA);
			}
		}
	}

	void Application::setCursorVisible(bool visible) {
		ShowCursor(visible);
	}

	bool Application::hasFocus() const {
		if (_win.wnd) return GetFocus() == _win.wnd;
		return false;
	}

	void Application::setFocus() {
		if (_win.wnd) SetFocus(_win.wnd);
	}

	bool Application::isMaximzed() const {
		if (_win.wnd) return _win.wndState == WindowState::MAXIMUM;
		return false;
	}

	void Application::setMaximum() {
		if (_win.wnd && _setWndState(WindowState::MAXIMUM)) {
			_win.wndDirty = true;
			if (!_isFullscreen && _isVisible) _updateWindowPlacement();
			_sendResizedEvent();
		}
	}

	bool Application::isMinimzed() const {
		if (_win.wnd) return _win.wndState == WindowState::MINIMUM;
		return false;
	}

	void Application::setMinimum() {
		if (_win.wnd && _setWndState(WindowState::MINIMUM)) {
			_win.wndDirty = true;
			if (!_isFullscreen && _isVisible) _updateWindowPlacement();
			_sendResizedEvent();
		}
	}

	void Application::setRestore() {
		if (_win.wnd && _setWndState(WindowState::NORMAL)) {
			_win.wndDirty = true;
			if (!_isFullscreen && _isVisible) _updateWindowPlacement();
			_sendResizedEvent();
		}
	}

	void Application::pollEvents() {
		MSG msg = { 0 };

		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				shutdown();
			} else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	bool Application::isVisible() const {
		return _win.wnd ? _isVisible : false;
	}

	void Application::setVisible(bool b) {
		if (_win.wnd && _isVisible != b) {
			_isVisible = b;
			_win.wndDirty = true;
			_updateWindowPlacement();
		}
	}

	void Application::shutdown() {
		if (!_isClosing) {
			_isClosing = true;
			_eventDispatcher->dispatchEvent(this, ApplicationEvent::CLOSED);
			std::exit(0);
		}
	}

	std::string_view Application::getAppId() const {
		return _appId;
	}

	const std::filesystem::path& Application::getAppPath() const {
		return _appPath;
	}

	//platform
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
			_eventDispatcher->dispatchEvent(this, ApplicationEvent::RESIZED);
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
				app->_eventDispatcher->dispatchEvent(app, ApplicationEvent::CLOSING, &isCanceled);
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
			if (app) app->_eventDispatcher->dispatchEvent(app, ApplicationEvent::FOCUS_IN);
			break;
		case WM_KILLFOCUS:
			if (app) app->_eventDispatcher->dispatchEvent(app, ApplicationEvent::FOCUS_OUT);
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
		case WM_INPUT:
			if (app) app->_eventDispatcher->dispatchEvent(app, ApplicationEvent::RAW_INPUT, &lParam);
			break;
		default:
			break;
		}

		return DefWindowProcW(hWnd, msg, wParam, lParam);
	}
}
#endif