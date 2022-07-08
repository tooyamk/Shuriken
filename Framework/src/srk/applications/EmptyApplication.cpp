#include "EmptyApplication.h"

namespace srk {
	EmptyApplication::EmptyApplication(const std::string_view& appId) :
		_isFullscreen(false),
		_isClosing(false),
		_isVisible(false),
		_appId(appId),
		_eventDispatcher(new events::EventDispatcher<ApplicationEvent>()) {
		_appPath = srk::getAppPath();
	}

	EmptyApplication::~EmptyApplication() {
	}

	IntrusivePtr<events::IEventDispatcher<ApplicationEvent>> EmptyApplication::getEventDispatcher() {
		return _eventDispatcher;
	}

	bool EmptyApplication::createWindow(const ApplicationStyle& style, const std::string_view& title, const Vec2ui32& clientSize, bool fullscreen) {
		if (_windowCreated) return false;

		_windowCreated = true;

		return true;
	}

	void* EmptyApplication::getNative(ApplicationNative native) const {
		return nullptr;
	}

	bool EmptyApplication::isFullscreen() const {
		return _isFullscreen;
	}

	Vec4ui32 EmptyApplication::getBorder() const {
		return _border;
	}

	void EmptyApplication::toggleFullscreen() {
	}

	Vec2ui32 EmptyApplication::getCurrentClientSize() const {
		return Vec2ui32();
	}

	Vec2ui32 EmptyApplication::getClientSize() const {
		return _clientSize;
	}

	void EmptyApplication::setClientSize(const Vec2ui32& size) {
		_clientSize = size;
	}

	void EmptyApplication::setWindowTitle(const std::string_view& title) {
	}

	void EmptyApplication::setWindowPosition(const Vec2i32& pos) {
	}

	void EmptyApplication::setCursorVisible(bool visible) {
	}

	bool EmptyApplication::hasFocus() const {
		return false;
	}

	void EmptyApplication::setFocus() {
	}

	bool EmptyApplication::isMaximzed() const {
		return false;
	}

	void EmptyApplication::setMaximum() {
	}

	bool EmptyApplication::isMinimzed() const {
		return false;
	}

	void EmptyApplication::setMinimum() {
	}

	void EmptyApplication::setRestore() {
	}

	void EmptyApplication::pollEvents() {
	}

	bool EmptyApplication::isVisible() const {
		return false;
	}

	void EmptyApplication::setVisible(bool b) {
	}

	void EmptyApplication::shutdown() {
		if (!_isClosing) {
			_isClosing = true;
			_eventDispatcher->dispatchEvent(this, ApplicationEvent::CLOSED);
			std::exit(0);
		}
	}

	std::string_view EmptyApplication::getAppId() const {
		return _appId;
	}

	const std::filesystem::path& EmptyApplication::getAppPath() const {
		return _appPath;
	}
}