#include "WindowWindows.h"

#if SRK_OS == SRK_OS_WINDOWS
#include "srk/String.h"
#include "srk/Debug.h"

namespace srk {
	Window::Window() :
		_eventDispatcher(new events::EventDispatcher<WindowEvent>()) {
		_data.sentSize.setAll((std::numeric_limits<decltype(_data.sentSize)::ElementType>::max)());
	}

	Window::~Window() {
		close();
	}

	std::atomic_uint32_t Window::_counter = 0;

	IntrusivePtr<events::IEventDispatcher<WindowEvent>> Window::getEventDispatcher() {
		return _eventDispatcher;
	}

	bool Window::create(const WindowStyle& style, const std::string_view& title, const Vec2ui32& clientSize, bool fullscreen) {
		if (_data.isCreated) return false;

		_data.clientSize = clientSize;
		_data.isFullscreen = fullscreen;
		_data.title = title;
		_data.module = GetModuleHandleW(nullptr);
		_data.style = style;
		_data.bkBrush = CreateSolidBrush(RGB(style.backgroundColor[0], style.backgroundColor[1], style.backgroundColor[2]));
		_data.className = String::Utf8ToUnicode(String::toString(GetCurrentProcessId()));
		_data.className += L'-';
		_data.className += String::Utf8ToUnicode(String::toString(_counter.fetch_add(1)));

		WNDCLASSEXW wnd;
		memset(&wnd, 0, sizeof(wnd));
		wnd.cbSize = sizeof(wnd);
		wnd.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wnd.lpfnWndProc = Window::_wndProc;
		wnd.cbClsExtra = 0;
		wnd.cbWndExtra = 0;
		wnd.hInstance = _data.module;
		wnd.hIcon = nullptr;//LoadIcon(NULL, IDI_APPLICATION);
		wnd.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wnd.hbrBackground = _data.bkBrush;
		wnd.lpszMenuName = nullptr;
		wnd.lpszClassName = _data.className.data();
		wnd.hIconSm = nullptr;

		RegisterClassExW(&wnd);

		_calcBorder();

		auto rect = _calcWindowRect(false);
		{
			_data.clinetPos.set((GetSystemMetrics(SM_CXSCREEN) - rect.size[0]) / 2 + _data.border[0], (GetSystemMetrics(SM_CYSCREEN) - rect.size[1]) / 2 + _data.border[2]);

			auto area = _calcWorkArea();
			_data.clinetPos.set(area.pos[0] + (area.size[0] - rect.size[0]) / 2 + _data.border[0], area.pos[1] + (area.size[1] - rect.size[1]) / 2 + _data.border[2]);

			if (_data.clinetPos[1] < GetSystemMetrics(SM_CYCAPTION)) _data.clinetPos[1] = GetSystemMetrics(SM_CYCAPTION);
		}

		if (_data.isFullscreen) {
			rect = _calcWindowRect(true);
		} else {
			rect.pos.set(_data.clinetPos[0], _data.clinetPos[1]);
		}

		_data.wnd = CreateWindowExW(_getWindowExStyle(_data.isFullscreen), wnd.lpszClassName, String::Utf8ToUnicode(title).data(), _getWindowStyle(_data.style, _data.isFullscreen),
			rect.pos[0], rect.pos[1], rect.size[0], rect.size[1],
			GetDesktopWindow(), nullptr, _data.module, nullptr);
		SetWindowLongPtr(_data.wnd, GWLP_USERDATA, (LONG_PTR)this);

		_data.isCreated = true;
		return true;
	}

	bool Window::isCreated() const {
		return _data.isCreated;
	}

	void* Window::getNative(WindowNative native) const {
		switch (native) {
		case WindowNative::MODULE:
			return _data.module;
		case WindowNative::WINDOW:
			return _data.wnd;
		}

		return nullptr;
	}

	bool Window::isFullscreen() const {
		return _data.isFullscreen;
	}

	Vec4ui32 Window::getBorder() const {
		return _data.border;
	}

	void Window::toggleFullscreen() {
		if (_data.wnd) {
			_data.isFullscreen = !_data.isFullscreen;
			_data.wndDirty = true;

			SetWindowLongPtrW(_data.wnd, GWL_STYLE, _getWindowStyle(_data.style, _data.isFullscreen));
			SetWindowLongPtrW(_data.wnd, GWL_EXSTYLE, _getWindowExStyle(_data.isFullscreen));

			if (_data.isVisible) {
				_data.ignoreEvtSize = true;
				_updateWindowPlacement();
				_data.ignoreEvtSize = false;
			}
			_sendResizedEvent();
		}
	}

	Vec2ui32 Window::getCurrentClientSize() const {
		Vec2ui32 size;

		if (_data.wnd) {
			if (_data.isVisible && _data.wndState != WindowState::MINIMUM) {
				RECT rect;
				GetClientRect(_data.wnd, &rect);
				size.set(rect.right - rect.left, rect.bottom - rect.top);
			} else {
				if (_data.isFullscreen) {
					size.set(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
				} else {
					if (_data.wndState == WindowState::MAXIMUM || (_data.wndState == WindowState::MINIMUM && _data.prevWndState == WindowState::MAXIMUM)) {
						auto area = _calcWorkArea();
						size.set(std::max<int32_t>(area.size[0], 0), std::max<int32_t>(area.size[1] - GetSystemMetrics(SM_CYCAPTION), 0));
					} else {
						size = _data.clientSize;
					}
				}
			}
		}

		return size;
	}

	Vec2ui32 Window::getClientSize() const {
		return _data.clientSize;
	}

	void Window::setClientSize(const Vec2ui32& size) {
		if (_data.wnd && _data.clientSize != size) {
			_data.clientSize = size;
			_data.wndDirty = true;

			if (!_data.isFullscreen && _data.isVisible && _data.wndState == WindowState::NORMAL) _updateWindowPlacement(SW_SHOWNA);
			_sendResizedEvent();
		}
	}

	std::string_view Window::getTitle() const {
		return _data.title;
	}

	void Window::setTitle(const std::string_view& title) {
		if (_data.wnd) {
			_data.title = title;
			SetWindowTextW(_data.wnd, String::Utf8ToUnicode(title).data());
		}
	}

	void Window::setPosition(const Vec2i32& pos) {
		if (!_data.wnd) return;
		
		if (auto p = pos + _data.border.components(0, 2); _data.clinetPos != p) {
			_data.clinetPos = p;
			_data.wndDirty = true;

			if (!_data.isFullscreen && _data.isVisible && _data.wndState == WindowState::NORMAL) _updateWindowPlacement(SW_SHOWNA);
		}
	}

	void Window::setCursorVisible(bool visible) {
		if (!_data.isCreated) return;
		ShowCursor(visible);
	}

	bool Window::hasFocus() const {
		if (_data.wnd) return GetFocus() == _data.wnd;
		return false;
	}

	void Window::setFocus() {
		if (_data.wnd) SetFocus(_data.wnd);
	}

	bool Window::isMaximzed() const {
		if (_data.wnd) return _data.wndState == WindowState::MAXIMUM;
		return false;
	}

	void Window::setMaximum() {
		if (_data.wnd && _setWndState(WindowState::MAXIMUM)) {
			_data.wndDirty = true;
			if (!_data.isFullscreen && _data.isVisible) _updateWindowPlacement();
			_sendResizedEvent();
		}
	}

	bool Window::isMinimzed() const {
		if (_data.wnd) return _data.wndState == WindowState::MINIMUM;
		return false;
	}

	void Window::setMinimum() {
		if (_data.wnd && _setWndState(WindowState::MINIMUM)) {
			_data.wndDirty = true;
			if (!_data.isFullscreen && _data.isVisible) _updateWindowPlacement();
			_sendResizedEvent();
		}
	}

	void Window::setRestore() {
		if (_data.wnd && _setWndState(WindowState::NORMAL)) {
			_data.wndDirty = true;
			if (!_data.isFullscreen && _data.isVisible) _updateWindowPlacement();
			_sendResizedEvent();
		}
	}

	bool Window::isVisible() const {
		return _data.wnd ? _data.isVisible : false;
	}

	void Window::setVisible(bool b) {
		if (_data.wnd && _data.isVisible != b) {
			_data.isVisible = b;
			_data.wndDirty = true;
			_updateWindowPlacement();
		}
	}

	void Window::pollEvents() {
		MSG msg = { 0 };
		while (_data.wnd && PeekMessage(&msg, _data.wnd, 0, 0, PM_REMOVE)) {
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
		if (!_data.isCreated) return;

		if (_data.wnd) DestroyWindow(_data.wnd);
		if (_data.bkBrush) DeleteObject(_data.bkBrush);
		if (!_data.className.empty()) UnregisterClassW(_data.className.data(), _data.module);

		_data = decltype(_data)();

		_eventDispatcher->dispatchEvent(this, WindowEvent::CLOSED);
	}

	//platform
	void Window::_calcBorder() {
		RECT rect = { 0, 0, 100, 100 };
		auto rst = AdjustWindowRectEx(&rect, _getWindowStyle(_data.style, false), FALSE, _getWindowExStyle(false));
		_data.border.set(-rect.left, rect.right - 100, -rect.top, rect.bottom - 100);
	}

	Box2i32 Window::_calcWindowRect(bool fullscreen) const {
		if (fullscreen) {
			return Box2i32(Vec2i32::ZERO, Vec2i32(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)));
		} else {
			return Box2i32(Vec2i32(_data.clinetPos[0] - _data.border[0], _data.clinetPos[1] - _data.border[2]), Vec2i32((Vec2i32::ElementType)_data.clientSize[0] + _data.border[0] + _data.border[1], (Vec2i32::ElementType)_data.clientSize[1] + _data.border[2] + _data.border[3]));
		}
	}

	void Window::_sendResizedEvent() {
		if (auto size = getCurrentClientSize(); _data.sentSize != size) {
			_data.sentSize = size;
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
		if (_data.wndState != state) {
			_data.prevWndState = _data.wndState;
			_data.wndState = state;
			return true;
		}

		return false;
	}

	void Window::_updateWindowPlacement(UINT showCmd) {
		if (showCmd == (std::numeric_limits<UINT>::max)()) {
			if (_data.isVisible) {
				if (_data.isFullscreen) {
					showCmd = SW_SHOWDEFAULT;
				} else {
					switch (_data.wndState) {
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
		GetWindowPlacement(_data.wnd, &info);

		if (_data.wndDirty || info.showCmd != showCmd) {
			_data.wndDirty = false;

			if (showCmd != (std::numeric_limits<decltype(showCmd)>::max)()) info.showCmd = showCmd;

			auto rect = _calcWindowRect(_data.isFullscreen);
			info.rcNormalPosition.left = rect.pos[0];
			info.rcNormalPosition.top = rect.pos[1];
			info.rcNormalPosition.right = rect.pos[0] + rect.size[0];
			info.rcNormalPosition.bottom = rect.pos[1] + rect.size[1];

			SetWindowPlacement(_data.wnd, &info);
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
				if (win->_data.ignoreEvtSize) return 0;

				if (!win->_data.isFullscreen) {
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

					if (wParam == SIZE_RESTORED) win->_data.clientSize.set(LOWORD(lParam), HIWORD(lParam));
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
			if (win && !win->_data.isFullscreen && !IsZoomed(win->_data.wnd) && !IsIconic(win->_data.wnd)) win->_data.clinetPos.set(LOWORD(lParam), HIWORD(lParam));
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