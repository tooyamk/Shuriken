#include "srk/predefine/OS.h"

#if SRK_OS == SRK_OS_LINUX

#ifndef SRK_HAS_X11
#	if __has_include(<X11/Xlib.h>)
#		define SRK_HAS_X11
#	endif
#endif

#ifdef SRK_HAS_X11
#	include <X11/Xlib.h>
#	include <X11/Xatom.h>
#	include <X11/Xutil.h>
#endif

#include "WindowLinux.h"

#include "srk/String.h"
#include "srk/Debug.h"

namespace srk {
#ifdef SRK_HAS_X11
	Window::Window() :
		_isFullscreen(false),
		_isVisible(false),
		_eventDispatcher(new events::EventDispatcher<WindowEvent>()) {
		_linux.sentSize.set((std::numeric_limits<decltype(_linux.sentSize)::ElementType>::max)());
	}

	Window::~Window() {
		close();
	}

	IntrusivePtr<Application> Window::getApplication() const {
		return _app;
	}

	IntrusivePtr<events::IEventDispatcher<WindowEvent>> Window::getEventDispatcher() {
		return _eventDispatcher;
	}

	bool Window::create(Application& app, const WindowStyle& style, const std::string_view& title, const Vec2ui32& clientSize, bool fullscreen) {
		if (_app) return false;

		_app = app;
		_clientSize = clientSize;
		_isFullscreen = fullscreen;
		_style = style;
		_isVisible = false;

		if (!_linux.dis) _linux.dis = XOpenDisplay(nullptr);
		if (!_linux.dis) {
			close();
			return false;
		}

		_linux.MOTIF_WM_HINTS = XInternAtom((Display*)_linux.dis, "_MOTIF_WM_HINTS", False);
		_linux.WM_STATE = XInternAtom((Display*)_linux.dis, "WM_STATE", False);
		_linux.WM_PROTOCOLS = XInternAtom((Display*)_linux.dis, "WM_PROTOCOLS", False);
		_linux.WM_DELETE_WINDOW = XInternAtom((Display*)_linux.dis, "WM_DELETE_WINDOW", False);
		_linux.NET_WM_PING = XInternAtom((Display*)_linux.dis, "_NET_WM_PING", False);
		_linux.NET_WM_WINDOW_TYPE = XInternAtom((Display*)_linux.dis, "_NET_WM_WINDOW_TYPE", False);
		_linux.NET_WM_WINDOW_TYPE_NORMAL = XInternAtom((Display*)_linux.dis, "_NET_WM_WINDOW_TYPE_NORMAL", False);
		_linux.NET_WM_STATE = XInternAtom((Display*)_linux.dis, "_NET_WM_STATE", False);
		_linux.NET_WM_STATE_FULLSCREEN = XInternAtom((Display*)_linux.dis, "_NET_WM_STATE_FULLSCREEN", False);
		_linux.NET_WM_STATE_MAXIMIZED_HORZ = XInternAtom((Display*)_linux.dis, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
		_linux.NET_WM_STATE_MAXIMIZED_VERT = XInternAtom((Display*)_linux.dis, "_NET_WM_STATE_MAXIMIZED_VERT", False);
		_linux.NET_REQUEST_FRAME_EXTENTS = XInternAtom((Display*)_linux.dis, "_NET_REQUEST_FRAME_EXTENTS", False);
		_linux.NET_FRAME_EXTENTS = XInternAtom((Display*)_linux.dis, "_NET_FRAME_EXTENTS", False);
		_linux.NET_WORKAREA = XInternAtom((Display*)_linux.dis, "_NET_WORKAREA", False);
		_linux.NET_CURRENT_DESKTOP = XInternAtom((Display*)_linux.dis, "_NET_CURRENT_DESKTOP", False);

		_linux.bgColor = _style.backgroundColor[0] << 16 | _style.backgroundColor[1] << 8 | _style.backgroundColor[2];

		XSetWindowAttributes attr = { 0 };
		attr.border_pixel = 0;
		//attr.background_pixel = _linux.bgColor;
		attr.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask |
			PointerMotionMask | ButtonPressMask | ButtonReleaseMask |
			ExposureMask | FocusChangeMask | VisibilityChangeMask |
			EnterWindowMask | LeaveWindowMask | PropertyChangeMask;

		_linux.screen = DefaultScreen((Display*)_linux.dis);
		_linux.root = RootWindow((Display*)_linux.dis, _linux.screen);
		_linux.wnd = XCreateWindow((Display*)_linux.dis, _linux.root,
			0, 0, _clientSize[0], _clientSize[1], 0,
			DefaultDepth((Display*)_linux.dis, 0), InputOutput, DefaultVisual((Display*)_linux.dis, 0), CWBorderPixel | CWColormap | CWEventMask, &attr);

		XSetWindowBackground((Display*)_linux.dis, _linux.wnd, _linux.bgColor);
		//XClearWindow((Display*)_linux.dis, _linux.wnd);

		//XSelectInput((Display*)_linux.dis, _linux.wnd, ExposureMask | ButtonPressMask | KeyPressMask | PropertyChangeMask);

		//auto sd = ScreenOfDisplay((Display*)_linux.dis, _linux.screen);
		//printcln(sd->width, "   ", sd->height);

		if (!_style.thickFrame) {
			auto hints = XAllocSizeHints();
			hints->flags = PMinSize | PMaxSize;
			hints->min_width = _clientSize[0];
			hints->min_height = _clientSize[1];
			hints->max_width = _clientSize[0];
			hints->max_height = _clientSize[1];
			XSetWMNormalHints((Display*)_linux.dis, _linux.wnd, hints);
			XFree(hints);
		}

		// Atom window_type = XInternAtom((Display*)_linux.dis, "_NET_WM_WINDOW_TYPE", False);
		// int64_t value = XInternAtom((Display*)_linux.dis, "_NET_WM_WINDOW_TYPE_NORMAL", False);
		// XChangeProperty((Display*)_linux.dis, _linux.wnd, window_type, XA_ATOM, 32, PropModeReplace, (uint8_t*)&value, 1);

		{
			Atom protocols[] = { _linux.WM_DELETE_WINDOW, _linux.NET_WM_PING };
			XSetWMProtocols((Display*)_linux.dis, _linux.wnd, protocols, sizeof(protocols) / sizeof(Atom));
		}

		{
			Atom type = _linux.NET_WM_WINDOW_TYPE_NORMAL;
			XChangeProperty((Display*)_linux.dis, _linux.wnd, _linux.NET_WM_WINDOW_TYPE, XA_ATOM, 32, PropModeReplace, (uint8_t*)&type, 1);
		}

		{
			MwmHints hints = { 0 };

			hints.flags = MwmHints::MWM_HINTS_FUNCTIONS | MwmHints::MWM_HINTS_DECORATIONS;
			hints.functions = MwmHints::MWM_FUNC_MOVE | MwmHints::MWM_FUNC_CLOSE | MwmHints::MWM_FUNC_RESIZE;
			hints.decorations = MwmHints::MWM_DECOR_BORDER | MwmHints::MWM_DECOR_RESIZEH | MwmHints::MWM_DECOR_TITLE | MwmHints::MWM_DECOR_MENU;
			if (_style.minimizeButton) {
				hints.functions |= MwmHints::MWM_FUNC_MINIMIZE;
				hints.decorations |= MwmHints::MWM_DECOR_MINIMIZE;
			}
			if (_style.maximizeButton) {
				hints.functions |= MwmHints::MWM_FUNC_MAXIMIZE;
				hints.decorations |= MwmHints::MWM_DECOR_MAXIMIZE;
			}

			XChangeProperty((Display*)_linux.dis, _linux.wnd, _linux.MOTIF_WM_HINTS, XA_ATOM, 32, PropModeReplace, (uint8_t*)&hints, 5);
		}

		{
			auto hints = XAllocWMHints();
			hints->flags = StateHint;
			hints->initial_state = NormalState;

			XSetWMHints((Display*)_linux.dis, _linux.wnd, hints);
			XFree(hints);
		}

		setTitle(title);

		XFlush((Display*)_linux.dis);

		{
			_linux.waitFrameEXTENTS = true;

			_sendClientEventToWM(_linux.NET_REQUEST_FRAME_EXTENTS);
			_waitEvent(_linux.waitFrameEXTENTS);
		}

		{
			auto area = _calcWorkArea();
			//auto sd = ScreenOfDisplay((Display*)_linux.dis, _linux.screen);
			_linux.wndPos.set((area.size[0] - _clientSize[0] - _border[0] - _border[1]) / 2, (area.size[1] - _clientSize[1] - _border[2] - _border[3]) / 2);

			if (auto top = area.pos[1] + _border[2]; _linux.wndPos[1] < top) _linux.wndPos[1] = top;
		}

		_updateWindowPlacement();

		return true;
	}

	void* Window::getNative(WindowNative native) const {
		switch (native) {
		case WindowNative::X_DISPLAY:
			return _linux.dis;
		case WindowNative::WINDOW:
			return (void*)_linux.wnd;
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
		if (_linux.wnd) {
			_isFullscreen = !_isFullscreen;
			_updateWindowPlacement();
		}
	}

	Vec2ui32 Window::getCurrentClientSize() const {
		Vec2ui32 size;

		if (_linux.wnd) {
			if (_isFullscreen) {
				auto sd = ScreenOfDisplay((Display*)_linux.dis, _linux.screen);
				size.set(sd->width, sd->height);
			}
			else {
				if (_linux.wndState == WindowState::MAXIMUM || (_linux.wndState == WindowState::MINIMUM && _linux.prevWndState == WindowState::MAXIMUM)) {
					auto area = _calcWorkArea();
					size.set(std::max<int32_t>(area.size[0], 0), std::max<int32_t>(area.size[1] - _border[2], 0));
				}
				else {
					size = _clientSize;
				}
			}
		}

		return size;
	}

	Vec2ui32 Window::getClientSize() const {
		return _clientSize;
	}

	void Window::setClientSize(const Vec2ui32& size) {
		if (_linux.wnd && _clientSize != size) {
			_clientSize = size;
			_updateWindowPlacement();
			_sendResizedEvent();
		}
	}

	std::string_view Window::getTitle() const {
		return _linux.title;
	}

	void Window::setTitle(const std::string_view& title) {
		if (_linux.wnd) {
			_linux.title = title;
			XStoreName((Display*)_linux.dis, _linux.wnd, _linux.title.data());
		}
	}

	void Window::setPosition(const Vec2i32& pos) {
		if (_linux.wnd && _linux.wndPos != pos) {
			_linux.wndPos = pos;
			_updateWindowPlacement();
		}
	}

	void Window::setCursorVisible(bool visible) {
		//todo
	}

	bool Window::hasFocus() const {
		if (!_linux.wnd) return false;

		::Window focused;
		int32_t revertTo;

		XGetInputFocus((Display*)_linux.dis, &focused, &revertTo);
		return focused == _linux.wnd;
	}

	void Window::setFocus() {
		if (_linux.wnd && _isVisible) XSetInputFocus((Display*)_linux.dis, _linux.wnd, RevertToParent, CurrentTime);
	}

	bool Window::isMaximzed() const {
		if (_linux.wnd) return _linux.wndState == WindowState::MAXIMUM;
		return false;
	}

	void Window::setMaximum() {
		if (_linux.wnd && _setWndState(WindowState::MAXIMUM)) _updateWindowPlacement();
	}

	bool Window::isMinimzed() const {
		if (_linux.wnd) return _linux.wndState == WindowState::MINIMUM;
		return false;
	}

	void Window::setMinimum() {
		if (_linux.wnd && _setWndState(WindowState::MINIMUM)) _updateWindowPlacement();
	}

	void Window::setRestore() {
		if (_linux.wnd && _setWndState(WindowState::NORMAL)) _updateWindowPlacement();
	}

	bool Window::isVisible() const {
		if (_linux.wnd) return _isVisible;
		return false;
	}

	void Window::setVisible(bool b) {
		if (_linux.wnd && _isVisible != b) {
			_isVisible = b;
			_updateWindowPlacement();
		}
	}

	void Window::pollEvents() {
		if (!_linux.dis) return;
		XEvent e = { 0 };
		while (XCheckIfEvent((Display*)_linux.dis, &e, [](Display* display, XEvent* event, XPointer pointer) { return True; }, (XPointer)this)) _doEvent(&e);
	}

	void Window::close() {
		if (!_app) return;

		if (_linux.wnd) {
			XDestroyWindow((Display*)_linux.dis, _linux.wnd);
			_linux.wnd = 0;
			_linux.title.clear();
		}

		if (_linux.dis) {
			XCloseDisplay((Display*)_linux.dis);
			_linux.dis = nullptr;
		}

		_isFullscreen = false;
		_clientSize = Vec2ui32();
		_border = Vec4i32();
		_isVisible = false;

		_app.reset();

		_eventDispatcher->dispatchEvent(this, WindowEvent::CLOSED);
	}

	//platform
	void Window::_sendClientEventToWM(X11_Atom msgType, int64_t a, int64_t b, int64_t c, int64_t d, int64_t e) {
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
		XSendEvent((Display*)_linux.dis, _linux.root, False, SubstructureNotifyMask | SubstructureRedirectMask, &evt);
	}

	void Window::_sendResizedEvent() {
		if (auto size = getCurrentClientSize(); _linux.sentSize != size) {
			_linux.sentSize = size;
			_eventDispatcher->dispatchEvent(this, WindowEvent::RESIZED);
		}
	}

	void Window::_waitEvent(bool& value) {
		while (value) {
			XEvent e = { 0 };
			if (XCheckIfEvent((Display*)_linux.dis, &e, [](Display* display, XEvent* event, XPointer pointer) { return True; }, (XPointer)this)) {
				_doEvent(&e);
			}
			else {
				value = false;
				break;
			}
		};
	}

	size_t Window::_getXWndProperty(X11_Window wnd, X11_Atom property, X11_Atom type, uint8_t** value) const {
		Atom actualType;
		int32_t actualFormat;
		uint64_t count, bytesAfter;

		XGetWindowProperty((Display*)_linux.dis, wnd, property, 0, (~0), False, type, &actualType, &actualFormat, &count, &bytesAfter, value);

		return count;
	}

	bool Window::_isMaximized() const {
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

	bool Window::_isMinimized() const {
		return _getXWndState() == IconicState;
	}

	int32_t Window::_getXWndState() const {
		int32_t result = WithdrawnState;
		struct {
			uint32_t state;
			Window icon;
		}*state = nullptr;

		if (_getXWndProperty(_linux.wnd, _linux.WM_STATE, _linux.WM_STATE, (uint8_t**)&state) >= 2) result = state->state;

		if (state) XFree(state);

		return result;
	}

	Box2i32 Window::_calcWorkArea() const {
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

	bool Window::_setWndState(WindowState state) {
		if (_linux.wndState != state) {
			_linux.prevWndState = _linux.wndState;
			_linux.wndState = state;
			return true;
		}

		return false;
	}

	void Window::_updateWindowPlacement() {
		if (_isVisible) {
			if (!_linux.xMapped) {
				_linux.xMapped = true;
				_linux.waitVisibility = true;
				XMapWindow((Display*)_linux.dis, _linux.wnd);
			}

			if (_isFullscreen) {
				if (!_linux.xFullscreen) {
					_linux.xFullscreen = true;
					_sendClientEventToWM(_linux.NET_WM_STATE, 1, _linux.NET_WM_STATE_FULLSCREEN, 0, 1);
				}
			}
			else {
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
						XIconifyWindow((Display*)_linux.dis, _linux.wnd, _linux.screen);
					}

					break;
				}
				default:
				{
					if (_linux.xWndState == WindowState::MINIMUM) {
						_linux.xWndState = WindowState::NORMAL;
						_linux.waitVisibility = true;
						XMapWindow((Display*)_linux.dis, _linux.wnd);
						_waitEvent(_linux.waitVisibility);
					}
					else if (_linux.xWndState == WindowState::MAXIMUM) {
						_sendClientEventToWM(_linux.NET_WM_STATE, 0, _linux.NET_WM_STATE_MAXIMIZED_VERT, _linux.NET_WM_STATE_MAXIMIZED_HORZ, 1);
					}

					XMoveWindow((Display*)_linux.dis, _linux.wnd, _linux.wndPos[0], _linux.wndPos[1]);

					break;
				}
				}
			}
		}
		else {
			if (_linux.xMapped) {
				_linux.xMapped = false;
				_linux.waitVisibility = true;
				XUnmapWindow((Display*)_linux.dis, _linux.wnd);
			}
		}

		_waitEvent(_linux.waitVisibility);
	}

	void Window::_doEvent(void* evt) {
		auto& e = *((XEvent*)evt);
		switch (e.type) {
		case ClientMessage:
		{
			auto msgType = e.xclient.message_type;

			if (msgType == _linux.WM_PROTOCOLS) {
				auto protocol = (Atom)e.xclient.data.l[0];

				if (protocol == _linux.WM_DELETE_WINDOW) {
					auto isCanceled = false;
					_eventDispatcher->dispatchEvent(this, WindowEvent::CLOSING, &isCanceled);
					if (!isCanceled) close();
				}
				else if (protocol == _linux.NET_WM_PING) {
					XEvent reply = e;
					reply.xclient.window = _linux.root;

					XSendEvent((Display*)_linux.dis, _linux.root, False, SubstructureNotifyMask | SubstructureRedirectMask, &reply);
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
				}
				else if (_isMinimized()) {
					_setWndState(WindowState::MINIMUM);
				}
				else {
					isNormal = true;
					_setWndState(WindowState::NORMAL);
				}
			}

			if (isNormal) {
				int32_t x, y;
				::Window dummy;
				XTranslateCoordinates((Display*)_linux.dis, _linux.wnd, _linux.root, 0, 0, &x, &y, &dummy);

				_linux.wndPos.set(x - _border[0], y - _border[2]);
				_clientSize.set(conf.width, conf.height);
			}

			_sendResizedEvent();

			break;
		}
		case Expose:
		{
			//XFlush((Display*)_linux.dis);
			//XSetWindowBackground((Display*)_linux.dis, _linux.wnd, _linux.bgColor);

			break;
		}
		case FocusIn:
		{
			auto mode = e.xfocus.mode;
			if (mode == NotifyGrab || mode == NotifyUngrab) return;

			_eventDispatcher->dispatchEvent(this, WindowEvent::FOCUS_IN);

			break;
		}
		case FocusOut:
		{
			auto mode = e.xfocus.mode;
			if (mode == NotifyGrab || mode == NotifyUngrab) return;

			_eventDispatcher->dispatchEvent(this, WindowEvent::FOCUS_OUT);

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
			}
			else if (atom == _linux.WM_STATE) {
				if (!_isFullscreen) {
					if (auto state = _getXWndState(); state == IconicState) {
						_setWndState(WindowState::MINIMUM);
					}
					else if (state == NormalState) {
						_setWndState(WindowState::NORMAL);
					}
				}
			}
			else if (atom == _linux.NET_WM_STATE) {
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
#endif