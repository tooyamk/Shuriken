#include "WindowMac.h"

#if SRK_OS == SRK_OS_MACOS

namespace srk {
	Window::Window() :
		_eventDispatcher(new events::EventDispatcher<WindowEvent>()) {
	}

	Window::~Window() {
		close();
	}

	IntrusivePtr<events::IEventDispatcher<WindowEvent>> Window::getEventDispatcher() {
		return _eventDispatcher;
	}

	bool Window::create(const WindowStyle& style, const std::string_view& title, const Vec2ui32& clientSize, bool fullscreen) {
		if (_data.isCreated) return false;

		_data.isCreated = true;
		return true;
	}

	bool Window::isCreated() const {
		return _data.isCreated;
	}

	void* Window::getNative(WindowNative native) const {
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

	void Window::setTitle(const std::string_view& title) {
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

	void Window::pollEvents() {
	}

	bool Window::isVisible() const {
		return false;
	}

	void Window::setVisible(bool b) {
	}

	void Window::close() {
		if (!_data.isCreated) return;

		_data = decltype(_data)();

		_eventDispatcher->dispatchEvent(this, WindowEvent::CLOSED);
	}
}
#endif