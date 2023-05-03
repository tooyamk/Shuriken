#include "Window.h"
#include "Manager.h"
//#include "srk/Printer.h"
#include "srk/StringUtility.h"
#include "srk/events/EventDispatcher.h"

namespace srk::modules::windows::android_native {
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
        _data.wnd = _manager->getNativeWindow();
		_data.sentContentSize = _manager->getNativeWindowSize();
        _data.hasFocus = _manager->hasFocus();

		_data.isCreated = true;
		setTitle(desc.title);

		_manager->add(extensions::AndroidNativeApplication::getInstance()->getActivity(), this);

		return true;
	}

    bool Window::isClosed() const {
        return !_data.isCreated;
	}

	void* Window::getNative(const std::string_view& native) const {
		using namespace std::string_view_literals;

        if (!_data.isCreated) return nullptr;

		if (native == "ANativeActivity"sv) return extensions::AndroidNativeApplication::getInstance()->getActivity();
        if (native == "ANativeWindow"sv) return _data.wnd;

		return nullptr;
	}

	bool Window::isFullScreen() const {
		return true;
	}

	Vec4ui32 Window::getFrameExtents() const {
		return Vec4ui32();
	}

	void Window::toggleFullScreen() {
	}

	Vec2ui32 Window::getContentSize() const {
        return _data.sentContentSize;
	}

	void Window::setContentSize(const Vec2ui32& size) {
	}

	std::string_view Window::getTitle() const {
		return _data.title;
	}

	void Window::setTitle(const std::string_view& title) {
        if (!_data.isCreated) return;
        _data.title = title;
	}

	void Window::setPosition(const Vec2i32& pos) {
	}

	void Window::setCursorVisible(bool visible) {
	}

	bool Window::hasFocus() const {
        return _data.hasFocus;
	}

	void Window::setFocus() {
	}

	bool Window::isMaximized() const {
        return true;
	}

	void Window::setMaximized() {
	}

	bool Window::isMinimized() const {
        return false;
	}

	void Window::setMinimized() {
	}

	void Window::setRestore() {
	}

	bool Window::isVisible() const {
        return true;
	}

	void Window::setVisible(bool b) {
	}

	void Window::close() {
		if (!_data.isCreated) return;

        if (this->getReferenceCount()) {
            _eventDispatcher->dispatchEvent(this, WindowEvent::CLOSING);
        } else {
            _eventDispatcher->dispatchEvent((void*)this, WindowEvent::CLOSED);
        }

		_manager->remove(extensions::AndroidNativeApplication::getInstance()->getActivity());
        
        //todo

		_data = decltype(_data)();

        if (this->getReferenceCount()) {
            _eventDispatcher->dispatchEvent(this, WindowEvent::CLOSED);
        } else {
            _eventDispatcher->dispatchEvent((void*)this, WindowEvent::CLOSED);
        }
	}

    void Window::processEvent(void* data) {
        auto& msg = *(Manager::Message*)data;
        switch (msg.type) {
        case extensions::AndroidNativeApplication::Event::WINDOW_CHANGED:
        {
            _data.wnd = msg.data.window;
            break;
        }
		case extensions::AndroidNativeApplication::Event::WINDOW_RESIZE:
			_sendResizedEvent(Vec2ui32(msg.data.size.width, msg.data.size.height));
            break;
		case extensions::AndroidNativeApplication::Event::FOCUS_CHANGED:
		{
			_data.hasFocus = msg.data.hasFocus;
            _eventDispatcher->dispatchEvent(this, msg.data.hasFocus ? WindowEvent::FOCUS_IN : WindowEvent::FOCUS_OUT);

			break;
		}
        }
    }

	//platform
	void Window::_sendResizedEvent(const Vec2ui32& size) {
		if (_data.sentContentSize != size) {
			_data.sentContentSize = size;
			_eventDispatcher->dispatchEvent(this, WindowEvent::RESIZED);
		}
	}
}