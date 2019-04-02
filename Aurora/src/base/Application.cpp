#include "Application.h"
#include "base/Time.h"
#include "utils/String.h"
#include <thread>

namespace aurora {
	Application::Application(const i8* appId, f64 frameInterval) :
		_appId(appId),
		_isClosing(false),
#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
		_hIns(nullptr),
		_hWnd(nullptr),
#endif
		_isWindowed(true),
		_time(0) {
		setFrameInterval(frameInterval);
	}

	Application::~Application() {
#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
		if (_hWnd) {
			DestroyWindow(_hWnd);
			_hWnd = nullptr;
		}

		if (_hIns) {
			auto appIdW = String::Utf8ToUnicode(_appId);
			UnregisterClass(appIdW.c_str(), _hIns);
			_hIns = nullptr;
		}
#endif
	}

	bool Application::createWindow(const Style& style, const std::string& title, const Rect<i32>& windowedRect, bool fullscreen) {
		_windowedRect.set(windowedRect);
		_isWindowed = !fullscreen;
		_style = style;

#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
		_hIns = GetModuleHandle(nullptr);

		WNDCLASSEXW wnd;
		memset(&wnd, 0, sizeof(WNDCLASSEXW));
		wnd.cbSize = sizeof(WNDCLASSEXW);
		wnd.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wnd.lpfnWndProc = Application::_wndProc;
		wnd.cbClsExtra = 0;
		wnd.cbWndExtra = 0;
		wnd.hInstance = _hIns;
		wnd.hIcon = nullptr;//LoadIcon(NULL, IDI_APPLICATION);
		wnd.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wnd.hbrBackground = nullptr;//static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
		wnd.lpszMenuName = nullptr;//NULL;
		auto appIdW = String::Utf8ToUnicode(_appId);
		wnd.lpszClassName = appIdW.c_str();
		wnd.hIconSm = nullptr;

		RegisterClassExW(&wnd);

		_adjustWindowRect(_windowedRect, _windowedRect);
		_updateWindowRectValue();

		_hWnd = CreateWindowExW(_getWindowExStyle(), wnd.lpszClassName, String::Utf8ToUnicode(title).c_str(), _getWindowStyle(),
			_wndRect.left, _wndRect.top, _wndRect.getWidth(), _wndRect.getHeight(),
			GetDesktopWindow(), nullptr, _hIns, nullptr);
		SetWindowLongPtr(_hWnd, GWL_USERDATA, (LONG_PTR)this);

		return _hWnd;
#endif
		return false;
	}

	bool Application::isWindowed() const {
		return _isWindowed;
	}

	void Application::toggleFullscreen() {
		_isWindowed = !_isWindowed;

		if (!_isWindowed) _recordWindowedRect();

#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
		_updateWindowRectValue();
		_changeWindow(true, true);
#endif
		_eventDispatcher.dispatchEvent(this, ApplicationEvent::RESIZE);
	}

	void Application::getInnerSize(i32& w, i32& h) {
#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
		RECT rect;
		GetClientRect(_hWnd, &rect);
		w = rect.right - rect.left;
		h = rect.bottom - rect.top;
#elif
		w = 0;
		h = 0;
#endif
	}

	void Application::getWindowedRect(Rect<i32>& dst) const {
#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
		if (_isWindowed) _recordWindowedRect();
#endif
		dst.set(_windowedRect);
	}

	void Application::setWindowedRect(const Rect<i32>& rect) {
		Rect<i32> out;
		_adjustWindowRect(rect, out);

		if (!_windowedRect.isEqual(out)) {
			bool isResize = _windowedRect.getWidth() != rect.getWidth() || _windowedRect.getHeight() != rect.getHeight();
			_windowedRect.set(rect);

			if (_isWindowed) {
#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
				_updateWindowRectValue();
				_changeWindow(false, true);
#endif
			}
		}
	}

	void Application::setWindowTitle(const std::string& title) {
#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
		if (_hWnd) SetWindowTextW(_hWnd, String::Utf8ToUnicode(title).c_str());
#endif
	}

	void Application::setCursorVisible(bool isVisible) {
#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
		ShowCursor(isVisible);
#endif
	}

	bool Application::hasFocus() const {
#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
		return GetForegroundWindow() == _hWnd;
#endif
	}

	void Application::pollEvents() {
#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
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
#endif
	}

	f64 Application::getFrameInterval() const {
		return _frameInterval;
	}

	void Application::setFrameInterval(f64 frameInterval) {
		_frameInterval = frameInterval <= 0. ? 0. : frameInterval;
	}

	void Application::resetDeltaRecord() {
		_time = 0;
	}

	void Application::setVisible(bool b) {
		if (_hWnd) {
			if (b) {
				ShowWindow(_hWnd, SW_SHOWDEFAULT);
			} else {
				ShowWindow(_hWnd, SW_HIDE);
			}

			UpdateWindow(_hWnd);
		}
	}

	void Application::run() {
		while (!_isClosing) {
			pollEvents();
			update(true);
		}
	}

	void Application::update(bool autoSleep) {
		auto t0 = Time::now<std::chrono::nanoseconds, std::chrono::steady_clock>();
		auto dt = _time == 0 ? 0 : (t0 - _time);
		_time = t0;

		_eventDispatcher.dispatchEvent(this, ApplicationEvent::UPDATE, &dt);

		if (autoSleep) {
			auto t1 = Time::now<std::chrono::nanoseconds, std::chrono::steady_clock>();

			f64 timePhase = f64(t1 - t0);
			if (timePhase < _frameInterval) std::this_thread::sleep_for(std::chrono::nanoseconds(i64(_frameInterval - timePhase)));
		}
	}

	void Application::shutdown() {
#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
		PostQuitMessage(0);
#endif
	}

	void Application::_adjustWindowRect(const Rect<i32>& in, Rect<i32>& out) {
#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
		RECT rect = { in.left, in.top, in.right, in.bottom };
		AdjustWindowRectEx(&rect, _getWindowStyle(), FALSE, _getWindowExStyle());
		out.set(rect.left, rect.top, rect.right, rect.bottom);
#elif
		out.set(in);
#endif
	}

	void Application::_recordWindowedRect() const {
#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
		RECT rect;
		GetWindowRect(_hWnd, &rect);
		_windowedRect.set(rect.left, rect.top, rect.right, rect.bottom);
#endif
	}

#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
	DWORD Application::_getWindowStyle() const {
		DWORD style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

		if (_isWindowed) {
			style |= WS_CAPTION | WS_SYSMENU;
			if (_style.minimizeButton) style |= WS_MINIMIZEBOX;
			if (_style.maximizeButton) style |= WS_MAXIMIZEBOX;
			if (_style.thickFrame) style |= WS_THICKFRAME;
		} else {
			style |= WS_POPUP;
		}

		return style;
	}

	DWORD Application::_getWindowExStyle() const {
		DWORD style = WS_EX_APPWINDOW;

		if (!_isWindowed) {
			//style |= WS_EX_TOPMOST;
		}

		return style;
	}

	void Application::_updateWindowRectValue() {
		if (_isWindowed) {
			_wndRect.set(_windowedRect);
		} else {
			_wndRect.set(0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
		}
	}

	void Application::_changeWindow(bool style, bool posOrSize) {
		if (style) SetWindowLongPtr(_hWnd, GWL_STYLE, _getWindowStyle());
		if (posOrSize) {
			ui32 flags = SWP_NOACTIVATE;
			if (_isWindowed) {
				flags |= SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOZORDER;
			} else {
				flags |= SWP_NOCOPYBITS;
			}
			SetWindowPos(_hWnd, HWND_NOTOPMOST, _wndRect.left, _wndRect.top, _wndRect.getWidth(), _wndRect.getHeight(), flags);
		}
	}

	LRESULT Application::_wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		auto app = (Application*)GetWindowLongPtr(hWnd, GWL_USERDATA);

		switch (msg) {
		case WM_CLOSE:
		{
			if (app) {
				bool isCanceled = false;
				app->_eventDispatcher.dispatchEvent(app, ApplicationEvent::CLOSING, &isCanceled);
				if (isCanceled) return 0;
			}

			break;
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			
			return 0;
		}
		case WM_SIZE:
		{
			if (app) app->_eventDispatcher.dispatchEvent(app, ApplicationEvent::RESIZE);

			break;
		}
		case WM_SETFOCUS:
		{
			if (app) app->_eventDispatcher.dispatchEvent(app, ApplicationEvent::FOCUS_IN);

			break;
		}
		case WM_KILLFOCUS:
		{
			if (app) app->_eventDispatcher.dispatchEvent(app, ApplicationEvent::FOCUS_OUT);

			break;
		}
		case WM_DEVICECHANGE:
		{
			break;
		}
		default:
			break;
		}

		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
#endif
}