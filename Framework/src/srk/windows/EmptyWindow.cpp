#include "EmptyWindow.h"

namespace srk {
	EmptyWindow::EmptyWindow() :
		_isClosing(false),
		_eventDispatcher(new events::EventDispatcher<WindowEvent>()) {
	}

	EmptyWindow::~EmptyWindow() {
		close();
	}

	IntrusivePtr<Application> EmptyWindow::getApplication() const {
		return _app;
	}

	IntrusivePtr<events::IEventDispatcher<WindowEvent>> EmptyWindow::getEventDispatcher() {
		return _eventDispatcher;
	}

	bool EmptyWindow::create(Application& app, const WindowStyle& style, const std::string_view& title, const Vec2ui32& clientSize, bool fullscreen) {
		if (_app) return false;

		_app = app;

		return true;
	}

	void* EmptyWindow::getNative(WindowNative native) const {
		return nullptr;
	}

	bool EmptyWindow::isFullscreen() const {
		return false;
	}

	Vec4ui32 EmptyWindow::getBorder() const {
		return Vec4ui32();
	}

	void EmptyWindow::toggleFullscreen() {
	}

	Vec2ui32 EmptyWindow::getCurrentClientSize() const {
		return Vec2ui32();
	}

	Vec2ui32 EmptyWindow::getClientSize() const {
		return Vec2ui32();
	}

	void EmptyWindow::setClientSize(const Vec2ui32& size) {
	}

	void EmptyWindow::setTitle(const std::string_view& title) {
	}

	void EmptyWindow::setPosition(const Vec2i32& pos) {
	}

	void EmptyWindow::setCursorVisible(bool visible) {
	}

	bool EmptyWindow::hasFocus() const {
		return false;
	}

	void EmptyWindow::setFocus() {
	}

	bool EmptyWindow::isMaximzed() const {
		return false;
	}

	void EmptyWindow::setMaximum() {
	}

	bool EmptyWindow::isMinimzed() const {
		return false;
	}

	void EmptyWindow::setMinimum() {
	}

	void EmptyWindow::setRestore() {
	}

	void EmptyWindow::pollEvents() {
	}

	bool EmptyWindow::isVisible() const {
		return false;
	}

	void EmptyWindow::setVisible(bool b) {
	}

	void EmptyWindow::close() {
		if (_app) _app.reset();
	}
}