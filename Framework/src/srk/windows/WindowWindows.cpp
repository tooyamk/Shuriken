#include "WindowWindows.h"

#if SRK_OS == SRK_OS_WINDOWS
#include "srk/String.h"
#include "srk/Debug.h"

namespace srk {
	Window::Window() :
		_isFullscreen(false),
		_isVisible(false),
		_eventDispatcher(new events::EventDispatcher<WindowEvent>()) {
		_win.sentSize.set((std::numeric_limits<decltype(_win.sentSize)::ElementType>::max)());
	}

	Window::~Window() {
		close();
	}

	std::atomic_uint32_t Window::_counter = 0;

	IntrusivePtr<Application> Window::getApplication() const {
		return _app;
	}

	IntrusivePtr<events::IEventDispatcher<WindowEvent>> Window::getEventDispatcher() {
		return _eventDispatcher;
	}

	//const events::IEventDispatcher<ApplicationEvent>& Application::getEventDispatcher() const {
	//	return _eventDispatcher;
	//}

	bool Window::create(Application& app, const WindowStyle& style, const std::string_view& title, const Vec2ui32& clientSize, bool fullscreen) {
		if (_app) return false;
		auto ins = (HINSTANCE)app.getNative();
		if (!ins) return false;

		_app = app;
		_clientSize = clientSize;
		_isFullscreen = fullscreen;
		_style = style;
		_isVisible = false;

		_win.bkBrush = CreateSolidBrush(RGB(style.backgroundColor[0], style.backgroundColor[1], style.backgroundColor[2]));

		WNDCLASSEXW wnd;
		memset(&wnd, 0, sizeof(wnd));
		wnd.cbSize = sizeof(wnd);
		wnd.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wnd.lpfnWndProc = Window::_wndProc;
		wnd.cbClsExtra = 0;
		wnd.cbWndExtra = 0;
		wnd.hInstance = ins;
		wnd.hIcon = nullptr;//LoadIcon(NULL, IDI_APPLICATION);
		wnd.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wnd.hbrBackground = _win.bkBrush;
		wnd.lpszMenuName = nullptr;
		_className = String::Utf8ToUnicode(_app->getApplicationId());
		_className += String::Utf8ToUnicode(String::toString(_counter.fetch_add(1)));
		wnd.lpszClassName = _className.data();
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
			GetDesktopWindow(), nullptr, ins, nullptr);
		SetWindowLongPtr(_win.wnd, GWLP_USERDATA, (LONG_PTR)this);

		_win.title = title;

		//DeleteObject(bkBrush);

		return true;
	}

	void* Window::getNative(WindowNative native) const {
		switch (native) {
		case WindowNative::WINDOW:
			return _win.wnd;
		}

		return nullptr;
	}

	bool Window::isFullscreen() const {
		return _isFullscreen;
	}

	Vec4ui32 Window::getBorder() const {
		return _border;
	}

	void Window::toggleFullscreen() {
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

	Vec2ui32 Window::getCurrentClientSize() const {
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

	Vec2ui32 Window::getClientSize() const {
		return _clientSize;
	}

	void Window::setClientSize(const Vec2ui32& size) {
		if (_win.wnd && _clientSize != size) {
			_clientSize = size;
			_win.wndDirty = true;

			if (!_isFullscreen && _isVisible && _win.wndState == WindowState::NORMAL) _updateWindowPlacement(SW_SHOWNA);
			_sendResizedEvent();
		}
	}

	std::string_view Window::getTitle() const {
		return _win.title;
	}

	void Window::setTitle(const std::string_view& title) {
		if (_win.wnd)
		{
			_win.title = title;
			SetWindowTextW(_win.wnd, String::Utf8ToUnicode(title).data());
		}
	}

	void Window::setPosition(const Vec2i32& pos) {
		if (_win.wnd) {
			if (auto p = pos + _border.components(0, 2); _win.clinetPos != p) {
				_win.clinetPos = p;
				_win.wndDirty = true;

				if (!_isFullscreen && _isVisible && _win.wndState == WindowState::NORMAL) _updateWindowPlacement(SW_SHOWNA);
			}
		}
	}

	void Window::setCursorVisible(bool visible) {
		if (!_app) return;
		ShowCursor(visible);
	}

	bool Window::hasFocus() const {
		if (_win.wnd) return GetFocus() == _win.wnd;
		return false;
	}

	void Window::setFocus() {
		if (_win.wnd) SetFocus(_win.wnd);
	}

	bool Window::isMaximzed() const {
		if (_win.wnd) return _win.wndState == WindowState::MAXIMUM;
		return false;
	}

	void Window::setMaximum() {
		if (_win.wnd && _setWndState(WindowState::MAXIMUM)) {
			_win.wndDirty = true;
			if (!_isFullscreen && _isVisible) _updateWindowPlacement();
			_sendResizedEvent();
		}
	}

	bool Window::isMinimzed() const {
		if (_win.wnd) return _win.wndState == WindowState::MINIMUM;
		return false;
	}

	void Window::setMinimum() {
		if (_win.wnd && _setWndState(WindowState::MINIMUM)) {
			_win.wndDirty = true;
			if (!_isFullscreen && _isVisible) _updateWindowPlacement();
			_sendResizedEvent();
		}
	}

	void Window::setRestore() {
		if (_win.wnd && _setWndState(WindowState::NORMAL)) {
			_win.wndDirty = true;
			if (!_isFullscreen && _isVisible) _updateWindowPlacement();
			_sendResizedEvent();
		}
	}

	bool Window::isVisible() const {
		return _win.wnd ? _isVisible : false;
	}

	void Window::setVisible(bool b) {
		if (_win.wnd && _isVisible != b) {
			_isVisible = b;
			_win.wndDirty = true;
			_updateWindowPlacement();
		}
	}

	void Window::pollEvents() {
		if (!_win.wnd) return;

		MSG msg = { 0 };

		while (PeekMessage(&msg, _win.wnd, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				close();
				break;
			} else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	void Window::close() {
		if (!_app) return;

		if (_win.wnd) {
			DestroyWindow(_win.wnd);
			_win.wnd = nullptr;
			_win.title.clear();
		}

		if (_win.bkBrush) {
			DeleteObject(_win.bkBrush);
			_win.bkBrush = nullptr;
		}

		if (!_className.empty()) {
			UnregisterClassW(_className.data(), (HINSTANCE)_app->getNative());
			_className = L"";
		}

		_isFullscreen = false;
		_clientSize = Vec2ui32();
		_border = Vec4i32();
		_isVisible = false;

		_app.reset();

		_eventDispatcher->dispatchEvent(this, WindowEvent::CLOSED);
	}

	//platform
	void Window::_calcBorder() {
		RECT rect = { 0, 0, 100, 100 };
		auto rst = AdjustWindowRectEx(&rect, _getWindowStyle(_style, false), FALSE, _getWindowExStyle(false));
		_border.set(-rect.left, rect.right - 100, -rect.top, rect.bottom - 100);
	}

	Box2i32 Window::_calcWindowRect(bool fullscreen) const {
		if (fullscreen) {
			return Box2i32(Vec2i32::ZERO, Vec2i32(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)));
		} else {
			return Box2i32(Vec2i32(_win.clinetPos[0] - _border[0], _win.clinetPos[1] - _border[2]), Vec2i32((Vec2i32::ElementType)_clientSize[0] + _border[0] + _border[1], (Vec2i32::ElementType)_clientSize[1] + _border[2] + _border[3]));
		}
	}

	void Window::_sendResizedEvent() {
		if (auto size = getCurrentClientSize(); _win.sentSize != size) {
			_win.sentSize = size;
			_eventDispatcher->dispatchEvent(this, WindowEvent::RESIZED);
		}
	}

	Box2i32 Window::_calcWorkArea() const {
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

	bool Window::_setWndState(WindowState state) {
		if (_win.wndState != state) {
			_win.prevWndState = _win.wndState;
			_win.wndState = state;
			return true;
		}

		return false;
	}

	void Window::_updateWindowPlacement(UINT showCmd) {
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

	DWORD Window::_getWindowStyle(const WindowStyle& style, bool fullscreen) {
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

	DWORD Window::_getWindowExStyle(bool fullscreen) {
		DWORD val = WS_EX_APPWINDOW;

		//if (fullscreen) val |= WS_EX_TOPMOST;

		return val;
	}

	LRESULT Window::_wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		auto win = (Window*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

		switch (msg) {
		case WM_CLOSE:
		{
			if (win) {
				auto isCanceled = false;
				win->_eventDispatcher->dispatchEvent(win, WindowEvent::CLOSING, &isCanceled);
				if (!isCanceled) win->close();
			}

			return 0;
		}
		case WM_DESTROY:
			//PostQuitMessage(0);
			return 0;
		case WM_SIZING:
			break;
		case WM_WINDOWPOSCHANGED:
			break;
		case WM_SIZE:
		{
			if (win) {
				if (win->_win.ignoreEvtSize) return 0;

				if (!win->_isFullscreen) {
					switch (wParam) {
					case SIZE_MINIMIZED:
						win->_setWndState(WindowState::MINIMUM);
						break;
					case SIZE_MAXIMIZED:
						win->_setWndState(WindowState::MAXIMUM);
						break;
					case SIZE_RESTORED:
						win->_setWndState(WindowState::NORMAL);
						break;
					default:
						break;
					}

					if (wParam == SIZE_RESTORED) win->_clientSize.set(LOWORD(lParam), HIWORD(lParam));
				}

				win->_sendResizedEvent();
			}

			break;
		}
		case WM_SETFOCUS:
			if (win) win->_eventDispatcher->dispatchEvent(win, WindowEvent::FOCUS_IN);
			break;
		case WM_KILLFOCUS:
			if (win) win->_eventDispatcher->dispatchEvent(win, WindowEvent::FOCUS_OUT);
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
			if (win && !win->_isFullscreen && !IsZoomed(win->_win.wnd) && !IsIconic(win->_win.wnd)) win->_win.clinetPos.set(LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_INPUT:
			if (win) win->_eventDispatcher->dispatchEvent(win, WindowEvent::RAW_INPUT, &lParam);
			break;
		default:
			break;
		}

		return DefWindowProcW(hWnd, msg, wParam, lParam);
	}
}
#endif