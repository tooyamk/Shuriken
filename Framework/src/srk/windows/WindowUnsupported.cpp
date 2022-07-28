#include "WindowUnsupported.h"

#if !defined(SRK_WINDOW_SUPPORTED)
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

		_data.isCreated = true;
		setTitle(title);

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

	Vec4ui32 Window::getFrameExtents() const {
		return Vec4ui32();
	}

	void Window::toggleFullscreen() {
	}

	Vec2ui32 Window::getCurrentContentSize() const {
		return Vec2ui32();
	}

	Vec2ui32 Window::getContentSize() const {
		return Vec2ui32();
	}

	void Window::setContentSize(const Vec2ui32& size) {
	}

	std::string_view Window::getTitle() const {
		return _data.title;
	}

	void Window::setTitle(const std::string_view& title) {
		if (_data.isCreated) _data.title = title;
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
		return false;
	}

	void Window::setVisible(bool b) {
	}

	void Window::close() {
		if (!_data.isCreated) return;

		_data = decltype(_data)();

		_eventDispatcher->dispatchEvent(this, WindowEvent::CLOSED);
	}

	void Window::processEvent(void* data) {
	}

	bool WindowManager::processEvent(const WindowManager::EventFn& fn) const {
		return false;
	}
}
#endif