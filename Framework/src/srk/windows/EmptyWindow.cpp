#include "EmptyWindow.h"

namespace srk {
	EmptyWindow::EmptyWindow() :
		_eventDispatcher(new events::EventDispatcher<WindowEvent>()) {
	}

	EmptyWindow::~EmptyWindow() {
		close();
	}

	IntrusivePtr<events::IEventDispatcher<WindowEvent>> EmptyWindow::getEventDispatcher() {
		return _eventDispatcher;
	}

	bool EmptyWindow::create(const WindowStyle& style, const std::string_view& title, const Vec2ui32& contentSize, bool fullscreen) {
		if (_data.isCreated) return false;

		_data.isCreated = true;
		setTitle(title);

		return true;
	}

	bool EmptyWindow::isCreated() const {
		return _data.isCreated;
	}

	void* EmptyWindow::getNative(WindowNative native) const {
		return nullptr;
	}

	bool EmptyWindow::isFullscreen() const {
		return false;
	}

	Vec4ui32 EmptyWindow::getFrameExtents() const {
		return Vec4ui32();
	}

	void EmptyWindow::toggleFullscreen() {
	}

	Vec2ui32 EmptyWindow::getCurrentContentSize() const {
		return Vec2ui32();
	}

	Vec2ui32 EmptyWindow::getContentSize() const {
		return Vec2ui32();
	}

	void EmptyWindow::setContentSize(const Vec2ui32& size) {
	}

	std::string_view EmptyWindow::getTitle() const {
		return _data.title;
	}

	void EmptyWindow::setTitle(const std::string_view& title) {
		if (_data.isCreated) _data.title = title;
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

	bool EmptyWindow::isVisible() const {
		return false;
	}

	void EmptyWindow::setVisible(bool b) {
	}

	void EmptyWindow::close() {
		if (!_data.isCreated) return;

		_data = decltype(_data)();

		_eventDispatcher->dispatchEvent(this, WindowEvent::CLOSED);
	}

	void EmptyWindow::processEvent(void* data) {
	}
}