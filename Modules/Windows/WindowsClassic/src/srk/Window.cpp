#include "Window.h"
#include "Manager.h"
#include "srk/String.h"
#include "srk/Debug.h"
#include "srk/events/EventDispatcher.h"

namespace srk::modules::windows::windows_classic {
	Window::Window(Manager& manager) :
		_manager(manager),
		_eventDispatcher(new events::EventDispatcher<WindowEvent>()) {
	}

	Window::~Window() {
		close();
	}

	std::atomic_uint32_t Window::_counter = 0;

	IntrusivePtr<events::IEventDispatcher<WindowEvent>> Window::getEventDispatcher() const {
		return _eventDispatcher;
	}

	bool Window::create(const CreateWindowDesc& desc) {
		if (_data.isCreated) return false;

		_data.contentRect.size = desc.contentSize;
		_data.isFullScreen = desc.fullScreen;
		_data.title = desc.title;
		_data.module = GetModuleHandleW(nullptr);
		_data.style = desc.style;
		_data.bkBrush = CreateSolidBrush(RGB(desc.style.backgroundColor[0], desc.style.backgroundColor[1], desc.style.backgroundColor[2]));
		_data.className = String::Utf8ToUnicode(String::toString(getCurrentProcessId()));
		_data.className += L'-';
		_data.className += String::Utf8ToUnicode(String::toString(_counter.fetch_add(1)));

		WNDCLASSEXW wnd;
		memset(&wnd, 0, sizeof(wnd));
		wnd.cbSize = sizeof(wnd);
		wnd.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		if (!desc.style.closable) wnd.style |= CS_NOCLOSE;
		wnd.lpfnWndProc = Window::_wndProc;
		wnd.cbClsExtra = 0;
		wnd.cbWndExtra = 0;
		wnd.hInstance = GetModuleHandleW(nullptr);
		wnd.hIcon = nullptr;//LoadIcon(NULL, IDI_APPLICATION);
		wnd.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wnd.hbrBackground = _data.bkBrush;
		wnd.lpszMenuName = nullptr;
		wnd.lpszClassName = _data.className.data();
		wnd.hIconSm = nullptr;

		RegisterClassExW(&wnd);

		_calcFrameExtends();

		auto rect = _calcFrameRect(false);
		{
			auto area = _calcWorkArea();
			_data.contentRect.pos.set(area.pos[0] + (area.size[0] - rect.size[0]) / 2 + _data.frameExtends[0], area.pos[1] + (area.size[1] - rect.size[1]) / 2 + _data.frameExtends[2]);

			if (_data.contentRect.pos[1] < GetSystemMetrics(SM_CYCAPTION)) _data.contentRect.pos[1] = GetSystemMetrics(SM_CYCAPTION);
		}

		if (_data.isFullScreen) {
			rect = _calcFrameRect(true);
		} else {
			rect.pos.set(_data.contentRect.pos[0], _data.contentRect.pos[1]);
		}

		_data.wnd = CreateWindowExW(_getNativeExStyle(_data.isFullScreen), wnd.lpszClassName, String::Utf8ToUnicode(desc.title).data(), _getNativeStyle(_data.style, _data.isFullScreen),
			rect.pos[0], rect.pos[1], rect.size[0], rect.size[1],
			GetDesktopWindow(), nullptr, _data.module, nullptr);
		SetWindowLongPtrW(_data.wnd, GWLP_USERDATA, (LONG_PTR)this);

		_data.isCreated = true;
		_data.sentContentSize = getCurrentContentSize();

		_manager->add(_data.wnd, this);

		return true;
	}

	bool Window::isValid() const {
		return _data.isCreated;
	}

	void* Window::getNative(WindowNative native) const {
		switch (native) {
		case WindowNative::WINDOW:
			return _data.wnd;
		}

		return nullptr;
	}

	bool Window::isFullScreen() const {
		return _data.isFullScreen;
	}

	void Window::toggleFullScreen() {
		if (_data.wnd) {
			_data.isFullScreen = !_data.isFullScreen;
			_data.wndDirty = true;

			SetWindowLongPtrW(_data.wnd, GWL_STYLE, _getNativeStyle(_data.style, _data.isFullScreen));
			SetWindowLongPtrW(_data.wnd, GWL_EXSTYLE, _getNativeExStyle(_data.isFullScreen));

			if (_data.isVisible) {
				_data.ignoreEvtSize = true;
				_updateWindowPlacement();
				_data.ignoreEvtSize = false;
			}
			_sendResizedEvent();
		}
	}

	Vec4ui32 Window::getFrameExtents() const {
		return _data.frameExtends;
	}

	Vec2ui32 Window::getCurrentContentSize() const {
		Vec2ui32 size;

		if (_data.wnd) {
			if (_data.isVisible && _data.wndState != WindowState::MINIMUM) {
				RECT rect;
				GetClientRect(_data.wnd, &rect);
				size.set(rect.right - rect.left, rect.bottom - rect.top);
			} else {
				if (_data.isFullScreen) {
					size.set(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
				} else {
					if (_data.wndState == WindowState::MAXIMUM || (_data.wndState == WindowState::MINIMUM && _data.prevWndState == WindowState::MAXIMUM)) {
						auto area = _calcWorkArea();
						size.set(std::max<int32_t>(area.size[0], 0), std::max<int32_t>(area.size[1] - GetSystemMetrics(SM_CYCAPTION), 0));
					} else {
						size = _data.contentRect.size;
					}
				}
			}
		}

		return size;
	}

	Vec2ui32 Window::getContentSize() const {
		return _data.contentRect.size;
	}

	void Window::setContentSize(const Vec2ui32& size) {
		if (!_data.wnd || _data.contentRect.size == size) return;

		_data.contentRect.size = size;
		_data.wndDirty = true;

		if (!_data.isFullScreen && _data.isVisible && _data.wndState == WindowState::NORMAL) _updateWindowPlacement(SW_SHOWNA);
		_sendResizedEvent();
	}

	std::string_view Window::getTitle() const {
		return _data.title;
	}

	void Window::setTitle(const std::string_view& title) {
		if (!_data.wnd) return;

		_data.title = title;
		SetWindowTextW(_data.wnd, String::Utf8ToUnicode(title).data());
	}

	void Window::setPosition(const Vec2i32& pos) {
		if (!_data.wnd) return;
		
		if (auto p = pos + _data.frameExtends.components(0, 2); _data.contentRect.pos != p) {
			_data.contentRect.pos = p;
			_data.wndDirty = true;

			if (!_data.isFullScreen && _data.isVisible && _data.wndState == WindowState::NORMAL) _updateWindowPlacement(SW_SHOWNA);
		}
	}

	void Window::setCursorVisible(bool visible) {
		if (!_data.isCreated) return;
		ShowCursor(visible);
	}

	bool Window::hasFocus() const {
		return _data.wnd ? GetFocus() == _data.wnd : false;
	}

	void Window::setFocus() {
		if (_data.wnd) SetFocus(_data.wnd);
	}

	bool Window::isMaximized() const {
		return _data.wnd ? _data.wndState == WindowState::MAXIMUM : false;
	}

	void Window::setMaximized() {
		if (_data.wnd && _setWndState(WindowState::MAXIMUM)) {
			_data.wndDirty = true;
			if (!_data.isFullScreen && _data.isVisible) _updateWindowPlacement();
			_sendResizedEvent();
		}
	}

	bool Window::isMinimized() const {
		return _data.wnd ? _data.wndState == WindowState::MINIMUM : false;
	}

	void Window::setMinimized() {
		if (_data.wnd && _setWndState(WindowState::MINIMUM)) {
			_data.wndDirty = true;
			if (!_data.isFullScreen && _data.isVisible) _updateWindowPlacement();
			_sendResizedEvent();
		}
	}

	void Window::setRestore() {
		if (_data.wnd && _setWndState(WindowState::NORMAL)) {
			_data.wndDirty = true;
			if (!_data.isFullScreen && _data.isVisible) _updateWindowPlacement();
			_sendResizedEvent();
		}
	}

	bool Window::isVisible() const {
		return _data.wnd ? _data.isVisible : false;
	}

	void Window::setVisible(bool b) {
		if (!_data.wnd || _data.isVisible == b) return;

		_data.isVisible = b;
		_data.wndDirty = true;
		_updateWindowPlacement();
	}

	void Window::close() {
		if (!_data.isCreated) return;

		_manager->remove(_data.wnd);

		if (_data.wnd) DestroyWindow(_data.wnd);
		if (_data.bkBrush) DeleteObject(_data.bkBrush);
		if (!_data.className.empty()) UnregisterClassW(_data.className.data(), _data.module);

		_data = decltype(_data)();

		if (this->getReferenceCount()) {
			_eventDispatcher->dispatchEvent(this, WindowEvent::CLOSED);
		} else {
			_eventDispatcher->dispatchEvent((void*)this, WindowEvent::CLOSED);
		}
	}

	void Window::processEvent(void* data) {
		if (!_data.isCreated) return;

		auto& msg =*(MSG*)data;
		if (msg.message == WM_QUIT) {
			close();
		} else {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	//platform
	void Window::_calcFrameExtends() {
		RECT rect = { 0, 0, 100, 100 };
		auto rst = AdjustWindowRectEx(&rect, _getNativeStyle(_data.style, false), FALSE, _getNativeExStyle(false));
		_data.frameExtends.set(-rect.left, rect.right - 100, -rect.top, rect.bottom - 100);
	}

	Box2i32 Window::_calcFrameRect(bool fullscreen) const {
		if (fullscreen) {
			return Box2i32(Vec2i32::ZERO, Vec2i32(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)));
		} else {
			auto& pos = _data.contentRect.pos;
			auto& size = _data.contentRect.size;
			auto& fe = _data.frameExtends;
			return Box2i32(Vec2i32(pos[0] - fe[0], pos[1] - fe[2]), Vec2i32((Vec2i32::ElementType)size[0] + fe[0] + fe[1], (Vec2i32::ElementType)size[1] + fe[2] + fe[3]));
		}
	}

	void Window::_sendResizedEvent() {
		if (auto size = getCurrentContentSize(); _data.sentContentSize != size) {
			_data.sentContentSize = size;
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
				if (_data.isFullScreen) {
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

			auto rect = _calcFrameRect(_data.isFullScreen);
			info.rcNormalPosition.left = rect.pos[0];
			info.rcNormalPosition.top = rect.pos[1];
			info.rcNormalPosition.right = rect.pos[0] + rect.size[0];
			info.rcNormalPosition.bottom = rect.pos[1] + rect.size[1];

			SetWindowPlacement(_data.wnd, &info);
		}
	}

	DWORD Window::_getNativeStyle(const WindowStyle& style, bool fullscreen) {
		DWORD val = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

		if (fullscreen) {
			val |= WS_POPUP;
		} else {
			val |= WS_BORDER | WS_DLGFRAME | WS_SYSMENU;
			if (style.minimizable) val |= WS_MINIMIZEBOX;
			if (style.maximizable) val |= WS_MAXIMIZEBOX;
			if (style.resizable) val |= WS_THICKFRAME;
		}

		return val;
	}

	DWORD Window::_getNativeExStyle(bool fullscreen) {
		DWORD val = WS_EX_APPWINDOW;

		//if (fullscreen) val |= WS_EX_TOPMOST;

		return val;
	}

	LRESULT Window::_wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		auto win = (Window*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);

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

				if (!win->_data.isFullScreen) {
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

					if (wParam == SIZE_RESTORED) win->_data.contentRect.size.set(LOWORD(lParam), HIWORD(lParam));
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
			if (win && !win->_data.isFullScreen && !IsZoomed(win->_data.wnd) && !IsIconic(win->_data.wnd)) win->_data.contentRect.pos.set(LOWORD(lParam), HIWORD(lParam));
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