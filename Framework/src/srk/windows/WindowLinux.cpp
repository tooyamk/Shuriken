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
		_eventDispatcher(new events::EventDispatcher<WindowEvent>()) {
		_data.sentSize.setAll((std::numeric_limits<decltype(_data.sentSize)::ElementType>::max)());
	}

	Window::~Window() {
		close();
	}

	IntrusivePtr<events::IEventDispatcher<WindowEvent>> Window::getEventDispatcher() {
		return _eventDispatcher;
	}

	bool Window::create(Application& app, const WindowStyle& style, const std::string_view& title, const Vec2ui32& clientSize, bool fullscreen) {
		if (_data.isCreated) return false;

		_data.clientSize = clientSize;
		_data.isFullscreen = fullscreen;
		_data.style = style;

		_data.dis = XOpenDisplay(nullptr);
		if (!_data.dis) {
			close();
			return false;
		}

		_data.MOTIF_WM_HINTS = XInternAtom((Display*)_data.dis, "_MOTIF_WM_HINTS", False);
		_data.WM_STATE = XInternAtom((Display*)_data.dis, "WM_STATE", False);
		_data.WM_PROTOCOLS = XInternAtom((Display*)_data.dis, "WM_PROTOCOLS", False);
		_data.WM_DELETE_WINDOW = XInternAtom((Display*)_data.dis, "WM_DELETE_WINDOW", False);
		_data.NET_WM_PING = XInternAtom((Display*)_data.dis, "_NET_WM_PING", False);
		_data.NET_WM_WINDOW_TYPE = XInternAtom((Display*)_data.dis, "_NET_WM_WINDOW_TYPE", False);
		_data.NET_WM_WINDOW_TYPE_NORMAL = XInternAtom((Display*)_data.dis, "_NET_WM_WINDOW_TYPE_NORMAL", False);
		_data.NET_WM_STATE = XInternAtom((Display*)_data.dis, "_NET_WM_STATE", False);
		_data.NET_WM_STATE_FULLSCREEN = XInternAtom((Display*)_data.dis, "_NET_WM_STATE_FULLSCREEN", False);
		_data.NET_WM_STATE_MAXIMIZED_HORZ = XInternAtom((Display*)_data.dis, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
		_data.NET_WM_STATE_MAXIMIZED_VERT = XInternAtom((Display*)_data.dis, "_NET_WM_STATE_MAXIMIZED_VERT", False);
		_data.NET_REQUEST_FRAME_EXTENTS = XInternAtom((Display*)_data.dis, "_NET_REQUEST_FRAME_EXTENTS", False);
		_data.NET_FRAME_EXTENTS = XInternAtom((Display*)_data.dis, "_NET_FRAME_EXTENTS", False);
		_data.NET_WORKAREA = XInternAtom((Display*)_data.dis, "_NET_WORKAREA", False);
		_data.NET_CURRENT_DESKTOP = XInternAtom((Display*)_data.dis, "_NET_CURRENT_DESKTOP", False);

		_data.bgColor = _data.style.backgroundColor[0] << 16 | _data.style.backgroundColor[1] << 8 | _data.style.backgroundColor[2];

		XSetWindowAttributes attr = { 0 };
		attr.border_pixel = 0;
		//attr.background_pixel = _linux.bgColor;
		attr.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask |
			PointerMotionMask | ButtonPressMask | ButtonReleaseMask |
			ExposureMask | FocusChangeMask | VisibilityChangeMask |
			EnterWindowMask | LeaveWindowMask | PropertyChangeMask;

		_data.screen = DefaultScreen((Display*)_data.dis);
		_data.root = RootWindow((Display*)_data.dis, _data.screen);
		_data.wnd = XCreateWindow((Display*)_data.dis, _data.root,
			0, 0, _data.clientSize[0], _data.clientSize[1], 0,
			DefaultDepth((Display*)_data.dis, 0), InputOutput, DefaultVisual((Display*)_data.dis, 0), CWBorderPixel | CWColormap | CWEventMask, &attr);

		XSetWindowBackground((Display*)_data.dis, _data.wnd, _data.bgColor);
		//XClearWindow((Display*)_linux.dis, _linux.wnd);

		//XSelectInput((Display*)_linux.dis, _linux.wnd, ExposureMask | ButtonPressMask | KeyPressMask | PropertyChangeMask);

		//auto sd = ScreenOfDisplay((Display*)_linux.dis, _linux.screen);
		//printcln(sd->width, "   ", sd->height);

		if (!_data.style.thickFrame) {
			auto hints = XAllocSizeHints();
			hints->flags = PMinSize | PMaxSize;
			hints->min_width = _clientSize[0];
			hints->min_height = _clientSize[1];
			hints->max_width = _clientSize[0];
			hints->max_height = _clientSize[1];
			XSetWMNormalHints((Display*)_data.dis, _data.wnd, hints);
			XFree(hints);
		}

		// Atom window_type = XInternAtom((Display*)_linux.dis, "_NET_WM_WINDOW_TYPE", False);
		// int64_t value = XInternAtom((Display*)_linux.dis, "_NET_WM_WINDOW_TYPE_NORMAL", False);
		// XChangeProperty((Display*)_linux.dis, _linux.wnd, window_type, XA_ATOM, 32, PropModeReplace, (uint8_t*)&value, 1);

		{
			Atom protocols[] = { _data.WM_DELETE_WINDOW, _data.NET_WM_PING };
			XSetWMProtocols((Display*)_data.dis, _data.wnd, protocols, sizeof(protocols) / sizeof(Atom));
		}

		{
			Atom type = _linux.NET_WM_WINDOW_TYPE_NORMAL;
			XChangeProperty((Display*)_data.dis, _data.wnd, _data.NET_WM_WINDOW_TYPE, XA_ATOM, 32, PropModeReplace, (uint8_t*)&type, 1);
		}

		{
			MwmHints hints = { 0 };

			hints.flags = MwmHints::MWM_HINTS_FUNCTIONS | MwmHints::MWM_HINTS_DECORATIONS;
			hints.functions = MwmHints::MWM_FUNC_MOVE | MwmHints::MWM_FUNC_CLOSE | MwmHints::MWM_FUNC_RESIZE;
			hints.decorations = MwmHints::MWM_DECOR_BORDER | MwmHints::MWM_DECOR_RESIZEH | MwmHints::MWM_DECOR_TITLE | MwmHints::MWM_DECOR_MENU;
			if (_data.style.minimizeButton) {
				hints.functions |= MwmHints::MWM_FUNC_MINIMIZE;
				hints.decorations |= MwmHints::MWM_DECOR_MINIMIZE;
			}
			if (_data.style.maximizeButton) {
				hints.functions |= MwmHints::MWM_FUNC_MAXIMIZE;
				hints.decorations |= MwmHints::MWM_DECOR_MAXIMIZE;
			}

			XChangeProperty((Display*)_data.dis, _data.wnd, _data.MOTIF_WM_HINTS, XA_ATOM, 32, PropModeReplace, (uint8_t*)&hints, 5);
		}

		{
			auto hints = XAllocWMHints();
			hints->flags = StateHint;
			hints->initial_state = NormalState;

			XSetWMHints((Display*)_data.dis, _data.wnd, hints);
			XFree(hints);
		}

		setTitle(title);

		XFlush((Display*)_data.dis);

		{
			_data.waitFrameEXTENTS = true;

			_sendClientEventToWM(_data.NET_REQUEST_FRAME_EXTENTS);
			_waitEvent(_data.waitFrameEXTENTS);
		}

		{
			auto area = _calcWorkArea();
			//auto sd = ScreenOfDisplay((Display*)_linux.dis, _linux.screen);
			_data.wndPos.set((area.size[0] - _data.clientSize[0] - _data.border[0] - _data.border[1]) / 2, (area.size[1] - _data.clientSize[1] - _data.border[2] - _data.border[3]) / 2);

			if (auto top = area.pos[1] + _border[2]; _data.wndPos[1] < top) _data.wndPos[1] = top;
		}

		_updateWindowPlacement();

		return true;
	}

	bool Window::isCreated() const {
		return _data.isCreated;
	}

	void* Window::getNative(WindowNative native) const {
		switch (native) {
		case WindowNative::X_DISPLAY:
			return _data.dis;
		case WindowNative::WINDOW:
			return (void*)_data.wnd;
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
			_updateWindowPlacement();
		}
	}

	Vec2ui32 Window::getCurrentClientSize() const {
		Vec2ui32 size;

		if (_data.wnd) {
			if (_data.isFullscreen) {
				auto sd = ScreenOfDisplay((Display*)_data.dis, _data.screen);
				size.set(sd->width, sd->height);
			} else {
				if (_data.wndState == WindowState::MAXIMUM || (_data.wndState == WindowState::MINIMUM && _data.prevWndState == WindowState::MAXIMUM)) {
					auto area = _calcWorkArea();
					size.set(std::max<int32_t>(area.size[0], 0), std::max<int32_t>(area.size[1] - _data.border[2], 0));
				} else {
					size = _data.clientSize;
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
			_updateWindowPlacement();
			_sendResizedEvent();
		}
	}

	std::string_view Window::getTitle() const {
		return _data.title;
	}

	void Window::setTitle(const std::string_view& title) {
		if (_data.wnd) {
			_data.title = title;
			XStoreName((Display*)_data.dis, _data.wnd, _data.title.data());
		}
	}

	void Window::setPosition(const Vec2i32& pos) {
		if (_data.wnd && _data.wndPos != pos) {
			_data.wndPos = pos;
			_updateWindowPlacement();
		}
	}

	void Window::setCursorVisible(bool visible) {
		//todo
	}

	bool Window::hasFocus() const {
		if (!_data.wnd) return false;

		::Window focused;
		int32_t revertTo;

		XGetInputFocus((Display*)_data.dis, &focused, &revertTo);
		return focused == _data.wnd;
	}

	void Window::setFocus() {
		if (_data.wnd && _data.isVisible) XSetInputFocus((Display*)_data.dis, _data.wnd, RevertToParent, CurrentTime);
	}

	bool Window::isMaximzed() const {
		if (_data.wnd) return _data.wndState == WindowState::MAXIMUM;
		return false;
	}

	void Window::setMaximum() {
		if (_data.wnd && _setWndState(WindowState::MAXIMUM)) _updateWindowPlacement();
	}

	bool Window::isMinimzed() const {
		if (_data.wnd) return _data.wndState == WindowState::MINIMUM;
		return false;
	}

	void Window::setMinimum() {
		if (_data.wnd && _setWndState(WindowState::MINIMUM)) _updateWindowPlacement();
	}

	void Window::setRestore() {
		if (_data.wnd && _setWndState(WindowState::NORMAL)) _updateWindowPlacement();
	}

	bool Window::isVisible() const {
		if (_data.wnd) return _data.isVisible;
		return false;
	}

	void Window::setVisible(bool b) {
		if (_data.wnd && _data.isVisible != b) {
			_data.isVisible = b;
			_updateWindowPlacement();
		}
	}

	void Window::pollEvents() {
		XEvent e = { 0 };
		while (_data.dis && XCheckIfEvent((Display*)_data.dis, &e, [](Display* display, XEvent* event, XPointer pointer) { return True; }, (XPointer)this)) _doEvent(&e);
	}

	void Window::close() {
		if (!_data.isCreated) return;

		if (_data.wnd) XDestroyWindow((Display*)_data.dis, _data.wnd);
		if (_data.dis) XCloseDisplay((Display*)_data.dis);

		_data = decltype(_data)();

		_eventDispatcher->dispatchEvent(this, WindowEvent::CLOSED);
	}

	//platform
	void Window::_sendClientEventToWM(X11_Atom msgType, int64_t a, int64_t b, int64_t c, int64_t d, int64_t e) {
		XEvent evt = { ClientMessage };
		auto& client = evt.xclient;
		client.window = _data.wnd;
		client.format = 32; // Data is 32-bit longs
		client.message_type = msgType;
		client.data.l[0] = a;
		client.data.l[1] = b;
		client.data.l[2] = c;
		client.data.l[3] = d;
		client.data.l[4] = e;
		XSendEvent((Display*)_data.dis, _data.root, False, SubstructureNotifyMask | SubstructureRedirectMask, &evt);
	}

	void Window::_sendResizedEvent() {
		if (auto size = getCurrentClientSize(); _data.sentSize != size) {
			_data.sentSize = size;
			_eventDispatcher->dispatchEvent(this, WindowEvent::RESIZED);
		}
	}

	void Window::_waitEvent(bool& value) {
		while (value) {
			XEvent e = { 0 };
			if (_data.dis && XCheckIfEvent((Display*)_data.dis, &e, [](Display* display, XEvent* event, XPointer pointer) { return True; }, (XPointer)this)) {
				_doEvent(&e);
			} else {
				value = false;
				break;
			}
		};
	}

	size_t Window::_getXWndProperty(X11_Window wnd, X11_Atom property, X11_Atom type, uint8_t** value) const {
		Atom actualType;
		int32_t actualFormat;
		uint64_t count, bytesAfter;

		XGetWindowProperty((Display*)_data.dis, wnd, property, 0, (~0), False, type, &actualType, &actualFormat, &count, &bytesAfter, value);

		return count;
	}

	bool Window::_isMaximized() const {
		Atom* states = nullptr;
		bool maximized = false;

		auto count = _getXWndProperty(_data.wnd, _data.NET_WM_STATE, XA_ATOM, (uint8_t**)&states);

		for (size_t i = 0; i < count; ++i) {
			if (states[i] == _data.NET_WM_STATE_MAXIMIZED_VERT || states[i] == _data.NET_WM_STATE_MAXIMIZED_HORZ) {
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

		if (_getXWndProperty(_data.wnd, _data.WM_STATE, _data.WM_STATE, (uint8_t**)&state) >= 2) result = state->state;

		if (state) XFree(state);

		return result;
	}

	Box2i32 Window::_calcWorkArea() const {
		Box2i32 area;

		Atom* extents = nullptr;
		Atom* desktop = nullptr;

		if (auto extentCount = _getXWndProperty(_data.root, _data.NET_WORKAREA, XA_CARDINAL, (uint8_t**)&extents); extentCount >= 4) {
			if (_getXWndProperty(_data.root, _data.NET_CURRENT_DESKTOP, XA_CARDINAL, (uint8_t**)&desktop) > 0) {
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
		if (_data.wndState != state) {
			_data.prevWndState = _data.wndState;
			_data.wndState = state;
			return true;
		}

		return false;
	}

	void Window::_updateWindowPlacement() {
		if (_data.isVisible) {
			if (!_data.xMapped) {
				_data.xMapped = true;
				_data.waitVisibility = true;
				XMapWindow((Display*)_data.dis, _data.wnd);
			}

			if (_data.isFullscreen) {
				if (!_data.xFullscreen) {
					_data.xFullscreen = true;
					_sendClientEventToWM(_data.NET_WM_STATE, 1, _data.NET_WM_STATE_FULLSCREEN, 0, 1);
				}
			} else {
				if (_data.xFullscreen) {
					_data.xFullscreen = false;
					_sendClientEventToWM(_data.NET_WM_STATE, 0, _data.NET_WM_STATE_FULLSCREEN, 0, 1);
				}

				switch (_data.wndState) {
				case WindowState::MAXIMUM:
				{
					if (_data.xWndState != WindowState::MAXIMUM) {
						_data.xWndState = WindowState::MAXIMUM;
						_sendClientEventToWM(_data.NET_WM_STATE, 1, _data.NET_WM_STATE_MAXIMIZED_VERT, _data.NET_WM_STATE_MAXIMIZED_HORZ, 1);
					}

					break;
				}
				case WindowState::MINIMUM:
				{
					if (_data.xWndState != WindowState::MINIMUM) {
						if (_data.xWndState == WindowState::MAXIMUM) {
							_sendClientEventToWM(_data.NET_WM_STATE, 0, _data.NET_WM_STATE_MAXIMIZED_VERT, _data.NET_WM_STATE_MAXIMIZED_HORZ, 1);
						}

						_data.xWndState = WindowState::MINIMUM;
						XIconifyWindow((Display*)_data.dis, _data.wnd, _data.screen);
					}

					break;
				}
				default:
				{
					if (_data.xWndState == WindowState::MINIMUM) {
						_data.xWndState = WindowState::NORMAL;
						_data.waitVisibility = true;
						XMapWindow((Display*)_data.dis, _data.wnd);
						_waitEvent(_data.waitVisibility);
					} else if (_data.xWndState == WindowState::MAXIMUM) {
						_sendClientEventToWM(_data.NET_WM_STATE, 0, _data.NET_WM_STATE_MAXIMIZED_VERT, _data.NET_WM_STATE_MAXIMIZED_HORZ, 1);
					}

					XMoveWindow((Display*)_data.dis, _data.wnd, _data.wndPos[0], _data.wndPos[1]);

					break;
				}
				}
			}
		} else {
			if (_data.xMapped) {
				_data.xMapped = false;
				_data.waitVisibility = true;
				XUnmapWindow((Display*)_data.dis, _data.wnd);
			}
		}

		_waitEvent(_data.waitVisibility);
	}

	void Window::_doEvent(void* evt) {
		auto& e = *((XEvent*)evt);
		switch (e.type) {
		case ClientMessage:
		{
			auto msgType = e.xclient.message_type;

			if (msgType == _data.WM_PROTOCOLS) {
				auto protocol = (Atom)e.xclient.data.l[0];

				if (protocol == _data.WM_DELETE_WINDOW) {
					auto isCanceled = false;
					_eventDispatcher->dispatchEvent(this, WindowEvent::CLOSING, &isCanceled);
					if (!isCanceled) close();
				} else if (protocol == _data.NET_WM_PING) {
					XEvent reply = e;
					reply.xclient.window = _data.root;

					XSendEvent((Display*)_data.dis, _data.root, False, SubstructureNotifyMask | SubstructureRedirectMask, &reply);
				}
			}

			break;
		}
		case ConfigureNotify:
		{
			auto isNormal = false;
			auto& conf = e.xconfigure;

			if (!_data.isFullscreen) {
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
				::Window dummy;
				XTranslateCoordinates((Display*)_data.dis, _data.wnd, _data.root, 0, 0, &x, &y, &dummy);

				_data.wndPos.set(x - _data.border[0], y - _data.border[2]);
				_data.clientSize.set(conf.width, conf.height);
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
			if (atom == _data.NET_FRAME_EXTENTS) {
				if (prop.state == PropertyNewValue && prop.window == _data.wnd) {
					uint8_t* property = nullptr;
					if (auto count = _getXWndProperty(_data.wnd, _data.NET_FRAME_EXTENTS, XA_CARDINAL, &property); count >= 4) {
						for (size_t i = 0; i < 4; ++i) _data.border[i] = ((int64_t*)property)[i];
					}

					if (property) XFree(property);

					_data.waitFrameEXTENTS = false;
				}
			} else if (atom == _data.WM_STATE) {
				if (!_data.isFullscreen) {
					if (auto state = _getXWndState(); state == IconicState) {
						_setWndState(WindowState::MINIMUM);
					} else if (state == NormalState) {
						_setWndState(WindowState::NORMAL);
					}
				}
			} else if (atom == _data.NET_WM_STATE) {
				if (!_data.isFullscreen && _isMaximized()) _setWndState(WindowState::MAXIMUM);
			}

			break;
		}
		case VisibilityNotify:
		{
			_data.waitVisibility = false;

			break;
		}
		default:
			break;
		}
	}
#endif
}
#endif