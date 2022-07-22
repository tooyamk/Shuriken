#include "WindowMac.h"

#if SRK_OS == SRK_OS_MACOS
#   import <Cocoa/Cocoa.h>

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

	bool Window::create(const WindowStyle& style, const std::string_view& title, const Vec2ui32& clientSize, bool fullscreen) {
		if (_data.isCreated) return false;
        
        NSUInteger styleMask = NSWindowStyleMaskTitled;
        if (style.minimizable) styleMask |= NSWindowStyleMaskMiniaturizable;
        if (style.closable) styleMask |=NSWindowStyleMaskClosable;
        if (style.resizable) styleMask |= NSWindowStyleMaskResizable;
        
        NSWindow* wnd = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, clientSize[0], clientSize[1]) styleMask:styleMask backing:NSBackingStoreBuffered defer:false];
        [wnd setBackgroundColor:[NSColor colorWithRed:style.backgroundColor[0] / 255.0f green:style.backgroundColor[1] / 255.0f blue:style.backgroundColor[2] / 255.0f alpha:1.0f]];
        _data.wnd = wnd;

		_data.isCreated = true;
        _register(wnd, this);
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

	Vec4ui32 Window::getBorder() const {
		return Vec4ui32();
	}

	void Window::toggleFullscreen() {
	}

	Vec2ui32 Window::getCurrentClientSize() const {
		return Vec2ui32();
	}

	Vec2ui32 Window::getClientSize() const {
		return Vec2ui32();
	}

	void Window::setClientSize(const Vec2ui32& size) {
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
	}

	void Window::setCursorVisible(bool visible) {
	}

	bool Window::hasFocus() const {
		return false;
	}

	void Window::setFocus() {
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
        
        _unregister(_data.wnd);
        if (_data.wnd) [(NSWindow*)_data.wnd release];

		_data = decltype(_data)();

		_eventDispatcher->dispatchEvent(this, WindowEvent::CLOSED);
	}

    void Window::processEvent(void* data) {
        if (!_data.isCreated) return;
        
        [(NSWindow*)_data.wnd sendEvent:(NSEvent*)data];
    }

    void WindowManager::pollEvents() {
        NSApplication* app = [NSApplication sharedApplication];
        do {
            NSEvent* e = [app nextEventMatchingMask:NSEventMaskAny untilDate:nil inMode:NSDefaultRunLoopMode dequeue:true];
            if (!e) break;
            //[app sendEvent:e];
            sendEvent(e.window, e);
            
            [e release];
        } while (true);
    }
}
#endif
