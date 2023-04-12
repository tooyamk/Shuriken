#include "Window.h"
#include "Manager.h"
#include "srk/String.h"
#include "srk/events/EventDispatcher.h"

@implementation SrkWindowDelegate
- (id)initWithProc:(void(*)(void*, srk::modules::windows::cocoa::Msg, void*))proc Target:(void*)target {
    if (self = [super init]) {
        _proc = proc;
        _target = target;
    }
    return self;
}

- (void)dealloc {
    [super dealloc];
    //NSLog(@"!!!!!!!!!!!!");
}

- (void)setTarget:(void*)target {
    _target = target;
}

- (void)windowDidBecomeMain:(NSNotification*)notification {
    //NSLog(@"windowDidBecomeMain");
    _proc(_target, srk::modules::windows::cocoa::Msg::FOCUS_IN, nullptr);
}

- (void)windowDidResignMain:(NSNotification*)notification {
    //NSLog(@"windowDidResignMain");
    _proc(_target, srk::modules::windows::cocoa::Msg::FOCUS_OUT, nullptr);
}

- (void)windowDidBecomeKey:(NSNotification*)notification {
    //NSLog(@"windowDidBecomeKey");
    //_proc(_target, SRK_WINMAC_FOCUS_IN, nullptr);
}

- (void)windowDidResignKey:(NSNotification*)notification {
    //NSLog(@"windowDidResignKey");
    //_proc(_target, SRK_WINMAC_FOCUS_OUT, nullptr);
}

- (void)windowDidResize:(NSNotification*)notification {
    //NSLog(@"windowDidResize");
    _proc(_target, srk::modules::windows::cocoa::Msg::RESIZED, nullptr);
}

- (BOOL)windowShouldClose:(NSWindow*)sender {
    bool isCanceled = false;
    _proc(_target, srk::modules::windows::cocoa::Msg::CLOSING, &isCanceled);
    return !isCanceled;
}

- (void)windowWillClose:(NSNotification*)notification {
    _proc(_target, srk::modules::windows::cocoa::Msg::CLOSED, nullptr);
}

- (void)windowDidEnterFullScreen:(NSNotification *)notification {
    //NSLog(@"windowDidEnterFullScreen");
    _proc(_target, srk::modules::windows::cocoa::Msg::ENTER_FULLSCREEN, nullptr);
}

- (void)windowDidExitFullScreen:(NSNotification *)notification {
    //NSLog(@"windowDidExitFullScreen");
    _proc(_target, srk::modules::windows::cocoa::Msg::EXIT_FULLSCREEN, nullptr);
}

@end

namespace srk::modules::windows::cocoa {
	Window::Window(Manager& manager) :
        _manager(manager),
		_eventDispatcher(new events::EventDispatcher<WindowEvent>()) {
	}

	Window::~Window() {
		close();
	}

	IntrusivePtr<events::IEventDispatcher<WindowEvent>> Window::getEventDispatcher() const {
		return _eventDispatcher;
	}

	bool Window::create(const CreateWindowDescriptor& desc) {
        NSUInteger styleMask = NSWindowStyleMaskTitled;
        if (desc.style.minimizable) styleMask |= NSWindowStyleMaskMiniaturizable;
        if (desc.style.closable) styleMask |=NSWindowStyleMaskClosable;
        if (desc.style.resizable) styleMask |= NSWindowStyleMaskResizable;
        
        auto wnd = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, desc.contentSize[0], desc.contentSize[1]) styleMask:styleMask backing:NSBackingStoreBuffered defer:false];
        [wnd setBackgroundColor:[NSColor colorWithRed:desc.style.backgroundColor[0] / 255.0f green:desc.style.backgroundColor[1] / 255.0f blue:desc.style.backgroundColor[2] / 255.0f alpha:1.0f]];
        auto delegate = [[SrkWindowDelegate alloc] initWithProc:_proc Target:this];
        //NSLog(@"1 retainCount : %lu", [delegate retainCount]);
        [wnd setDelegate:delegate];
        [wnd center];
        //[wnd makeKeyAndOrderFront:nil];
        //delegate = [wnd delegate];
        //NSLog(@"2 retainCount : %lu", [delegate retainCount]);
        //[delegate release];
        //NSLog(@"3 retainCount : %lu", [delegate retainCount]);
        _data.wnd = wnd;
        _data.delegate = delegate;

		_data.isCreated = true;
		_data.sentContentSize = getContentSize();
		setTitle(desc.title);

		_manager->add(_data.wnd, this);

		return true;
	}

    bool Window::isValid() const {
		return _data.isCreated;
	}

	void* Window::getNative(const std::string_view& native) const {
        using namespace std::string_view_literals;

		if (native == "NSWindow"sv) return _data.wnd;
		return nullptr;
	}

	bool Window::isFullScreen() const {
		return _data.wnd ? _data.fullScreen : false;
	}

	Vec4ui32 Window::getFrameExtents() const {
        Vec4ui32 extents;
        
        if (_data.wnd) {
            auto rect = [NSWindow frameRectForContentRect:NSMakeRect(0, 0, 0, 0) styleMask:((NSWindow*)_data.wnd).styleMask];
            extents[0] = -rect.origin.x;
            extents[1] = rect.size.width + rect.origin.x;
            extents[2] = rect.size.height + rect.origin.y;
            extents[3] = -rect.origin.y;
        }
		return extents;
	}

	void Window::toggleFullScreen() {
        if (!_data.isCreated) return;
        
        [(NSWindow*)_data.wnd toggleFullScreen:nil];
	}

	Vec2ui32 Window::getContentSize() const {
		Vec2ui32 size;
        
        if (_data.wnd) {
            auto rect = [(NSWindow*)_data.wnd contentLayoutRect];
            size.set(rect.size.width, rect.size.height);
        }
        
        return size;
	}

	void Window::setContentSize(const Vec2ui32& size) {
        if (!_data.isCreated) return;

        NSSize s;
        s.width = size[0];
        s.height = size[1];
        [(NSWindow*)_data.wnd setContentSize:s];
	}

	std::string_view Window::getTitle() const {
		return _data.title;
	}

	void Window::setTitle(const std::string_view& title) {
        if (!_data.isCreated) return;

        _data.title = title;
        ((NSWindow*)_data.wnd).title = [NSString stringWithCString:_data.title.c_str() encoding:[NSString defaultCStringEncoding]];
	}

	void Window::setPosition(const Vec2i32& pos) {
        if (!_data.isCreated || _data.fullScreen || isMinimized() || isMaximized()) return;
        
        auto wnd = (NSWindow*)_data.wnd;
        [wnd setFrameOrigin:NSMakePoint(pos[0], wnd.screen.frame.size.height - (wnd.frame.size.height + pos[1]))];
	}

	void Window::setCursorVisible(bool visible) {
	}

	bool Window::hasFocus() const {
        return _data.wnd ? [(NSWindow*)_data.wnd isMainWindow] : false;
	}

	void Window::setFocus() {
        if (!_data.isCreated) return;

        auto wnd = (NSWindow*)_data.wnd;
        if (![wnd canBecomeMainWindow]) return;
        
        [NSApp activateIgnoringOtherApps:true];
        [wnd makeMainWindow];
	}

	bool Window::isMaximized() const {
        return _data.wnd ? ((NSWindow*)_data.wnd).zoomed : false;
	}

	void Window::setMaximized() {
        if (!_data.wnd) return;
        
        auto wnd = (NSWindow*)_data.wnd;
        [wnd performZoom:wnd];
	}

	bool Window::isMinimized() const {
        return _data.wnd ? ((NSWindow*)_data.wnd).miniaturized : false;
	}

	void Window::setMinimized() {
        if (!_data.wnd) return;
        
        auto wnd = (NSWindow*)_data.wnd;
        [wnd performMiniaturize:wnd];
	}

	void Window::setRestore() {
        if (!_data.wnd) return;
        
        auto wnd = (NSWindow*)_data.wnd;
        if (wnd.miniaturized) {
            [wnd deminiaturize:wnd];
        } else if (wnd.zoomed) {
            [wnd zoom:wnd];
        }
	}

	bool Window::isVisible() const {
        return _data.wnd ? [(NSWindow*)_data.wnd isVisible] : false;
	}

	void Window::setVisible(bool b) {
        if (!_data.wnd) return;
        
        auto wnd = (NSWindow*)_data.wnd;
        if ([wnd isVisible] == b) return;
        [wnd setIsVisible:b];
	}

	void Window::close() {
		if (!_data.isCreated) return;

        if (this->getReferenceCount()) {
            _eventDispatcher->dispatchEvent(this, WindowEvent::CLOSING);
        } else {
            _eventDispatcher->dispatchEvent((void*)this, WindowEvent::CLOSED);
        }

		_manager->remove(_data.wnd);
        
        if (_data.delegate) {
            auto delegate = (SrkWindowDelegate*)_data.delegate;
            [delegate setTarget:nullptr];
            [delegate release];
        }
        if (_data.wnd) {
            //NSLog(@"close");
            auto wnd = (NSWindow*)_data.wnd;
            //auto delegate = [wnd delegate];
            //NSLog(@"4 retainCount : %lu", [delegate retainCount]);
            //[wnd setDelegate:nil];
            //NSLog(@"5 retainCount : %lu", [delegate retainCount]);
            //[delegate release];
            //NSLog(@"6 retainCount : %lu", [delegate retainCount]);
            //NSLog(@"w1 retainCount : %lu", [wnd retainCount]);
            [wnd close];
            //NSLog(@"w2 retainCount : %lu", [wnd retainCount]);
            [wnd release];
            //NSLog(@"w3 retainCount : %lu", [wnd retainCount]);
        }

		_data = decltype(_data)();

        if (this->getReferenceCount()) {
            _eventDispatcher->dispatchEvent(this, WindowEvent::CLOSED);
        } else {
            _eventDispatcher->dispatchEvent((void*)this, WindowEvent::CLOSED);
        }
	}

    void Window::processEvent(void* data) {
        //if (!_data.isCreated) return;
        [NSApp sendEvent:(NSEvent*)data];
        //[(NSWindow*)_data.wnd sendEvent:(NSEvent*)data];
    }

    //platform
    void Window::_sendResizedEvent() {
        if (auto size = getContentSize(); _data.sentContentSize != size) {
            _data.sentContentSize = size;
            _eventDispatcher->dispatchEvent(this, WindowEvent::RESIZED);
        }
    }

    void Window::_proc(void* target, Msg msg, void* param) {
        auto win = (Window*)target;
        if (!win) return;
        
        switch (msg) {
            case Msg::FOCUS_IN:
                win->_eventDispatcher->dispatchEvent(win, WindowEvent::FOCUS_IN);
                break;
            case Msg::FOCUS_OUT:
                win->_eventDispatcher->dispatchEvent(win, WindowEvent::FOCUS_OUT);
                break;
            case Msg::CLOSING:
                win->_eventDispatcher->dispatchEvent(win, WindowEvent::CLOSING, param);
                break;
            case Msg::CLOSED:
                win->close();
                break;
            case Msg::RESIZED:
                win->_sendResizedEvent();
                break;
            case Msg::ENTER_FULLSCREEN:
                win->_data.fullScreen = true;
                break;
            case Msg::EXIT_FULLSCREEN:
                win->_data.fullScreen = false;
                break;
        }
    }
}
