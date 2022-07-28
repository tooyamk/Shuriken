#include "WindowMac.h"

#if SRK_OS == SRK_OS_MACOS
#   import <Cocoa/Cocoa.h>
#   include "srk/Debug.h"
#	include "srk/windows/WindowManager.h"

#ifndef SRK_WINMAC_MSG
#   define SRK_WINMAC_MSG

#   define SRK_WINMAC_CLOSING 1
#   define SRK_WINMAC_CLOSED 2
#   define SRK_WINMAC_FOCUS_IN 3
#   define SRK_WINMAC_FOCUS_OUT 4
#   define SRK_WINMAC_RESIZED 5
#endif

@interface SrkWindowDelegate : NSObject<NSWindowDelegate> {
    @private
    void(*_proc)(void*, uint32_t, void*);
    void* _target;
}
- (id)initWithProc:(void(*)(void*, uint32_t, void*))proc Target:(void*)target;
- (void)setTarget:(void*)target;
@end

@implementation SrkWindowDelegate
- (id)initWithProc:(void(*)(void*, uint32_t, void*))proc Target:(void*)target {
    if (self = [super init]) {
        _proc = proc;
        _target = target;
    }
    return self;
}

- (void)dealloc {
    [super dealloc];
    NSLog(@"!!!!!!!!!!!!");
}

- (void)setTarget:(void*)target {
    _target = target;
}

- (void)windowDidBecomeMain:(NSNotification*)notification {
    NSLog(@"windowDidBecomeMain");
    _proc(_target, SRK_WINMAC_FOCUS_IN, nullptr);
}

- (void)windowDidResignMain:(NSNotification*)notification {
    NSLog(@"windowDidResignMain");
    _proc(_target, SRK_WINMAC_FOCUS_OUT, nullptr);
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
    _proc(_target, SRK_WINMAC_RESIZED, nullptr);
}

- (BOOL)windowShouldClose:(NSWindow*)sender {
    bool isCanceled = false;
    _proc(_target, SRK_WINMAC_CLOSING, &isCanceled);
    return !isCanceled;
}

- (void)windowWillClose:(NSNotification*)notification {
    _proc(_target, SRK_WINMAC_CLOSED, nullptr);
}

@end

namespace srk {
	Window::Window() :
		_eventDispatcher(new events::EventDispatcher<WindowEvent>()) {
	}

	Window::~Window() {
		close();
	}

	WindowManager* Window::_manager = new WindowManager();

	IntrusivePtr<events::IEventDispatcher<WindowEvent>> Window::getEventDispatcher() {
		return _eventDispatcher;
	}

	bool Window::create(const WindowStyle& style, const std::string_view& title, const Vec2ui32& contentSize, bool fullscreen) {
		if (_data.isCreated) return false;
        
        NSUInteger styleMask = NSWindowStyleMaskTitled;
        if (style.minimizable) styleMask |= NSWindowStyleMaskMiniaturizable;
        if (style.closable) styleMask |=NSWindowStyleMaskClosable;
        if (style.resizable) styleMask |= NSWindowStyleMaskResizable;
        
        auto wnd = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, contentSize[0], contentSize[1]) styleMask:styleMask backing:NSBackingStoreBuffered defer:false];
        [wnd setBackgroundColor:[NSColor colorWithRed:style.backgroundColor[0] / 255.0f green:style.backgroundColor[1] / 255.0f blue:style.backgroundColor[2] / 255.0f alpha:1.0f]];
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
		_data.sentContentSize = getCurrentContentSize();
        _manager->add(wnd, this);
		setTitle(title);

		return true;
	}

	bool Window::isCreated() const {
		return _data.isCreated;
	}

	void* Window::getNative(WindowNative native) const {
        switch (native) {
            case WindowNative::WINDOW:
                return _data.wnd;
        }
        
		return nullptr;
	}

	bool Window::isFullscreen() const {
		return false;
	}

	Vec4ui32 Window::getFrameExtents() const {
		return Vec4ui32();
	}

	void Window::toggleFullscreen() {
        if (!_data.isCreated) return;
        
        [(NSWindow*)_data.wnd toggleFullScreen:nil];
	}

	Vec2ui32 Window::getCurrentContentSize() const {
        Vec2ui32 size;
        
        if (_data.wnd) {
            auto rect = [(NSWindow*)_data.wnd contentLayoutRect];
            size.set(rect.size.width, rect.size.height);
        }
        
        return size;
	}

	Vec2ui32 Window::getContentSize() const {
		return Vec2ui32();
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
        if (!_data.isCreated) return;
        
        NSPoint p;
        p.x = pos[0];
        p.y = pos[1];
        [(NSWindow*)_data.wnd setFrameOrigin:p];
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

	bool Window::isMaximzed() const {
		return false;
	}

	void Window::setMaximum() {
	}

	bool Window::isMinimzed() const {
		return false;
	}

	void Window::setMinimum() {
	}

	void Window::setRestore() {
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

		_eventDispatcher->dispatchEvent(this, WindowEvent::CLOSED);
	}

    void Window::processEvent(void* data) {
        //if (!_data.isCreated) return;
        [NSApp sendEvent:(NSEvent*)data];
        //[(NSWindow*)_data.wnd sendEvent:(NSEvent*)data];
    }

    bool WindowManager::processEvent(const WindowManager::EventFn& fn) const {
        //[NSApp finishLaunching];
        auto e = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:nil inMode:NSDefaultRunLoopMode dequeue:true];
        if (!e) return false;

        if (sendEvent(e.window, e, fn)) [NSApp sendEvent:e];
        [e release];
        return true;
    }

    //platform
    void Window::_sendResizedEvent() {
        if (auto size = getCurrentContentSize(); _data.sentContentSize != size) {
            _data.sentContentSize = size;
            _eventDispatcher->dispatchEvent(this, WindowEvent::RESIZED);
        }
    }

    void Window::_proc(void* target, uint32_t msg, void* param) {
        auto win = (Window*)target;
        if (!win) return;
        
        switch (msg) {
            case SRK_WINMAC_FOCUS_IN:
                win->_eventDispatcher->dispatchEvent(win, WindowEvent::FOCUS_IN);
                break;
            case SRK_WINMAC_FOCUS_OUT:
                win->_eventDispatcher->dispatchEvent(win, WindowEvent::FOCUS_OUT);
                break;
            case SRK_WINMAC_CLOSING:
                win->_eventDispatcher->dispatchEvent(win, WindowEvent::CLOSING, param);
                break;
            case SRK_WINMAC_CLOSED:
                win->close();
                break;
            case SRK_WINMAC_RESIZED:
                win->_sendResizedEvent();
                break;
        }
    }
}
#endif
