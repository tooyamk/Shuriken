#include "Application.h"
#include "base/String.h"
#include "base/Time.h"
#include <thread>

namespace aurora {
	Application::Application(const i8* appId, f64 frameInterval) :
		_appId(appId),
		_isClosing(false),
#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
		_hIns(nullptr),
		_hWnd(nullptr),
		_lastWndInnerRect({0, 0, 0, 0}),
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

	bool Application::createWindow(const Style& style, const std::string& title, const Box2i32& windowedRect, bool fullscreen) {
		_windowedRect.set(windowedRect);
		_isWindowed = !fullscreen;
		_style = style;

#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
		_hIns = GetModuleHandle(nullptr);

		WNDCLASSEXW wnd;
		memset(&wnd, 0, sizeof(wnd));
		wnd.cbSize = sizeof(wnd);
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
			_wndRect.pos[0], _wndRect.pos[1], _wndRect.size[0], _wndRect.size[1],
			GetDesktopWindow(), nullptr, _hIns, nullptr);
		SetWindowLongPtr(_hWnd, GWLP_USERDATA, (LONG_PTR)this);
		if (_hWnd) GetClientRect(_hWnd, &_lastWndInnerRect);

		return _hWnd;
#endif
		return false;
	}

	bool Application::isWindowed() const {
		return _isWindowed;
	}

	void Application::toggleFullscreen() {
		if (_hWnd) {
			_isWindowed = !_isWindowed;

			bool visibled = isVisible();

			if (!_isWindowed) {
				GetClientRect(_hWnd, &_lastWndInnerRect);
				_recordWindowedRect();
			}

#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
			_updateWindowRectValue();
			_changeWindow(true, true);
			if (visibled) ShowWindow(_hWnd, SW_SHOWDEFAULT);
#endif
			
			_eventDispatcher.dispatchEvent(this, ApplicationEvent::RESIZED);
		}
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

	void Application::getWindowedRect(Box2i32& dst) const {
#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
		if (_isWindowed) _recordWindowedRect();
#endif
		dst.set(_windowedRect);
	}

	void Application::setWindowedRect(const Box2i32& rect) {
		Box2i32 out;
		if (_adjustWindowRect(rect, out)) {
			_lastWndInnerRect.left = rect.pos[0];
			_lastWndInnerRect.right = rect.pos[0] + rect.size[0];
			_lastWndInnerRect.top = rect.pos[1];
			_lastWndInnerRect.bottom = rect.pos[1] + rect.size[1];

			if (!_windowedRect.isEqual(out)) {
				bool isResize = _windowedRect.size != rect.size;
				_windowedRect.set(rect);

				if (_isWindowed) {
#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
					_updateWindowRectValue();
					_changeWindow(false, true);
#endif
				}
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

	bool Application::isVisible() const {
#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
		return _hWnd ? IsWindowVisible(_hWnd) : false;
#endif
		return false;
	}

	void Application::setVisible(bool b) {
#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
		if (_hWnd) {
			if (b) {
				ShowWindow(_hWnd, SW_SHOWDEFAULT);
			} else {
				ShowWindow(_hWnd, SW_HIDE);
			}

			UpdateWindow(_hWnd);
		}
#endif
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

	const std::string& Application::getAppPath() const {
#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
		if (_appPath.empty()) {
			WCHAR wpath[1024];
			GetModuleFileNameW(nullptr, wpath, sizeof(wpath));
			std::string path = String::UnicodeToUtf8(wpath);
			i8 cDir[500] = "";
			i8 cDrive[100] = "";
			i8 cf[100] = "";
			i8 cExt[50] = "";
			_splitpath_s(path.c_str(), cDrive, cDir, cf, cExt);

			_appPath = cDrive;
			_appPath += cDir;
		}
#endif
		return _appPath;
	}

	bool Application::_adjustWindowRect(const Box2i32& in, Box2i32& out) {
#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
		RECT rect = { in.pos[0], in.pos[1], in.pos[0] + in.size[0], in.pos[1] + in.size[1] };
		auto rst = AdjustWindowRectEx(&rect, _getWindowStyle(), FALSE, _getWindowExStyle());
		out.set(Vec2i32({ rect.left, rect.top }), Vec2i32({ rect.right - rect.left, rect.bottom - rect.top }));
		return rst;
#elif
		out.set(in);
#endif
	}

	void Application::_recordWindowedRect() const {
#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
		RECT rect;
		GetWindowRect(_hWnd, &rect);
		_windowedRect.set(Vec2i32({ rect.left, rect.top }), Vec2i32({ rect.right - rect.left, rect.bottom - rect.top }));
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
			_wndRect.set(Vec2i32::ZERO, Vec2i32({ GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) }));
		}
	}

	void Application::_changeWindow(bool style, bool posOrSize) {
		if (style) {
			SetWindowLongPtr(_hWnd, GWL_STYLE, _getWindowStyle());
			SetWindowLongPtr(_hWnd, GWL_EXSTYLE, _getWindowExStyle());
		}
		if (posOrSize) {
			ui32 flags = SWP_NOACTIVATE;
			if (_isWindowed) {
				flags |= SWP_NOOWNERZORDER | SWP_NOZORDER;
			} else {
				flags |= SWP_NOCOPYBITS;
			}
			SetWindowPos(_hWnd, HWND_NOTOPMOST, _wndRect.pos[0], _wndRect.pos[1], _wndRect.size[0], _wndRect.size[1], flags);
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
			if (app) app->_eventDispatcher.dispatchEvent(app, ApplicationEvent::RESIZED);

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