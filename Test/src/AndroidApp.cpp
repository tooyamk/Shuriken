#include "AndroidApp.h"

#if SRK_OS == SRK_OS_ANDROID
#include "srk/Printer.h"
#include "srk/events/EventDispatcher.h"
#include "srk/modules/windows/WindowModule.h"

namespace srk::extensions {
    AndroidNativeApplication::AndroidNativeApplication(ANativeActivity* activity, ALooper* looper, bool shared) :
        _activity(activity),
        _window(nullptr),
        _inputQueue(nullptr),
        _looper(looper),
        _hasFocus(false),
        _state(State::UNKNOWN),
        _eventDispatcher(new events::EventDispatcher<Event>()) {

        if (!_looper) _looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);

        if (!shared) {
            auto callbacks = _activity->callbacks;
            callbacks->onStart = _onStart;
            callbacks->onResume = _onResume;
            callbacks->onSaveInstanceState = _onSaveInstanceState;
            callbacks->onPause = _onPause;
            callbacks->onStop = _onStop;
            callbacks->onDestroy = _onDestroy;
            callbacks->onWindowFocusChanged = _onWindowFocusChanged;
            callbacks->onNativeWindowCreated = _onNativeWindowCreated;
            callbacks->onNativeWindowResized = _onNativeWindowResized;
            callbacks->onNativeWindowRedrawNeeded = _onNativeWindowRedrawNeeded;
            callbacks->onNativeWindowDestroyed = _onNativeWindowDestroyed;
            callbacks->onInputQueueCreated = _onInputQueueCreated;
            callbacks->onInputQueueDestroyed = _onInputQueueDestroyed;
            callbacks->onContentRectChanged = _onContentRectChanged;
            callbacks->onConfigurationChanged = _onConfigurationChanged;
            callbacks->onLowMemory = _onLowMemory;

            _activity->instance = this;
        }
    }

    AndroidNativeApplication::~AndroidNativeApplication() {

    }

    AndroidNativeApplication* AndroidNativeApplication::_ins = nullptr;

    void AndroidNativeApplication::init(ANativeActivity* activity, ALooper* looper, bool shared) {
        using namespace std::string_view_literals;

        printaln(L"init"sv);
        _ins = new AndroidNativeApplication(activity, looper, shared);
    }

    void AndroidNativeApplication::setState(State state) {
        using namespace std::string_view_literals;

        std::scoped_lock lock(_mutex);

        if (_state != state) {
            _state = state;
            printaln(L"state changed "sv, (uint8_t)_state);
            _eventDispatcher->dispatchEvent(this, Event::STATE_CHANGED, &_state);
        }
    }

    void AndroidNativeApplication::setWindow(ANativeWindow* window) {
        using namespace std::string_view_literals;

        std::scoped_lock lock(_mutex);

        if (_window != window) {
            _eventDispatcher->dispatchEvent(this, Event::WINDOW_CHANGING, _window);
            _window = window;
            printaln(L"window changed "sv, _window);
            _eventDispatcher->dispatchEvent(this, Event::WINDOW_CHANGED, _window);
        }
    }

    void AndroidNativeApplication::setInputQueue(AInputQueue* queue) {
        std::scoped_lock lock(_mutex);

        if (_inputQueue != queue) {
            _eventDispatcher->dispatchEvent(this, Event::INPUT_QUEUE_CHANGING, _inputQueue);
            _inputQueue = queue;
            _eventDispatcher->dispatchEvent(this, Event::INPUT_QUEUE_CHANGED, _inputQueue);
        }
    }

    void* AndroidNativeApplication::onSaveInstanceState(size_t* outSize) {
        return nullptr;
    }

    void AndroidNativeApplication::onDestroy() {

    }

    void AndroidNativeApplication::setFocus(bool hasFocus) {
        using namespace std::string_view_literals;

        std::scoped_lock lock(_mutex);

        if (_hasFocus != hasFocus) {
            _hasFocus = hasFocus;
            printaln(L"focus changed "sv, _hasFocus);
            _eventDispatcher->dispatchEvent(this, Event::FOCUS_CHANGED, &_hasFocus);
        }
    }

    void AndroidNativeApplication::onWindowResized(ANativeWindow* window) {

    }

    void AndroidNativeApplication::onWindowRedrawNeeded(ANativeWindow* window) {

    }

    void AndroidNativeApplication::onContentRectChanged(const ARect* rect) {

    }

    void AndroidNativeApplication::onConfigurationChanged() {

    }

    void AndroidNativeApplication::lowMemory() {
        std::scoped_lock lock(_mutex);

        _eventDispatcher->dispatchEvent(this, Event::LOW_MEMORY);
    }

    std::filesystem::path AndroidNativeApplication::getAppPath() {
        auto env = _activity->env;

        auto clazz = env->GetObjectClass(_activity->clazz);
        auto methodID = env->GetMethodID(clazz, "getPackageCodePath", "()Ljava/lang/String;");
        auto result = env->CallObjectMethod(_activity->clazz, methodID);
        jboolean isCopy;
        auto strBuf = env->GetStringUTFChars((jstring)result, &isCopy);
        auto strLen = env->GetStringLength((jstring)result);
        std::filesystem::path path(std::string_view(strBuf, strLen));
        env->ReleaseStringUTFChars((jstring)result, strBuf);
        
        return std::move(path);
    }

    void AndroidNativeApplication::_onStart(ANativeActivity* activity) {
        _getApp(activity)->setState(State::STARTED);
    }

    void AndroidNativeApplication::_onResume(ANativeActivity* activity) {
        _getApp(activity)->setState(State::RESUMED);
    }

    void* AndroidNativeApplication::_onSaveInstanceState(ANativeActivity* activity, size_t* outSize) {
        return _getApp(activity)->onSaveInstanceState(outSize);
    }

    void AndroidNativeApplication::_onPause(ANativeActivity* activity) {
        _getApp(activity)->setState(State::PAUSED);
    }

    void AndroidNativeApplication::_onStop(ANativeActivity* activity) {
        _getApp(activity)->setState(State::STOPPED);
    }

    void AndroidNativeApplication::_onDestroy(ANativeActivity* activity) {
        _getApp(activity)->onDestroy();
    }

    void AndroidNativeApplication::_onWindowFocusChanged(ANativeActivity* activity, int hasFocus) {
        _getApp(activity)->setFocus(hasFocus != 0);
    }

    void AndroidNativeApplication::_onNativeWindowCreated(ANativeActivity* activity, ANativeWindow* window) {
        _getApp(activity)->setWindow(window);
    }

    void AndroidNativeApplication::_onNativeWindowResized(ANativeActivity* activity, ANativeWindow* window) {
        _getApp(activity)->onWindowResized(window);
    }

    void AndroidNativeApplication::_onNativeWindowRedrawNeeded(ANativeActivity* activity, ANativeWindow* window) {
        _getApp(activity)->onWindowRedrawNeeded(window);
    }

    void AndroidNativeApplication::_onNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow* window) {
        _getApp(activity)->setWindow(nullptr);
    }

    void AndroidNativeApplication::_onInputQueueCreated(ANativeActivity* activity, AInputQueue* queue) {
        _getApp(activity)->setInputQueue(queue);
    }

    void AndroidNativeApplication::_onInputQueueDestroyed(ANativeActivity* activity, AInputQueue* queue) {
        _getApp(activity)->setInputQueue(nullptr);
    }

    void AndroidNativeApplication::_onContentRectChanged(ANativeActivity* activity, const ARect* rect) {
        _getApp(activity)->onContentRectChanged(rect);
    }

    void AndroidNativeApplication::_onConfigurationChanged(ANativeActivity* activity) {
        _getApp(activity)->onConfigurationChanged();
    }

    void AndroidNativeApplication::_onLowMemory(ANativeActivity* activity) {
        _getApp(activity)->lowMemory();
    }
}

namespace srk::modules::windows::android_native {
    Manager::Manager() :
        _window(nullptr) {
	}

	Manager::~Manager() {
	}

	IntrusivePtr<IWindow> Manager::crerate(const CreateWindowDescriptor& desc) {
        if (_window) return false;

		auto win = new Window(*this);
		if (!win->create(desc)) {
			delete win;
			win = nullptr;
		}
        _window = win;
        
		return win;
	}

	bool Manager::processEvent(const IWindowModule::EventFn& fn) const {
		return true;
	}

    //=============================

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
		_data.isCreated = true;
		_data.sentContentSize = getContentSize();
		setTitle(desc.title);

		_manager->add(_data.wnd, this);

		return true;
	}

    bool Window::isClosed() const {
		return false;
	}

	void* Window::getNative(const std::string_view& native) const {
		return nullptr;
	}

	bool Window::isFullScreen() const {
		return true;
	}

	Vec4ui32 Window::getFrameExtents() const {
        Vec4ui32 extents;
		return extents;
	}

	void Window::toggleFullScreen() {
	}

	Vec2ui32 Window::getContentSize() const {
		Vec2ui32 size;
        
        return size;
	}

	void Window::setContentSize(const Vec2ui32& size) {
	}

	std::string_view Window::getTitle() const {
		return _data.title;
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

	bool Window::isMaximized() const {
        return false;
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
        return false;
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

		_manager->remove(_data.wnd);
        
        //todo

		_data = decltype(_data)();

        if (this->getReferenceCount()) {
            _eventDispatcher->dispatchEvent(this, WindowEvent::CLOSED);
        } else {
            _eventDispatcher->dispatchEvent((void*)this, WindowEvent::CLOSED);
        }
	}

    void Window::processEvent(void* data) {
    }
}
#endif