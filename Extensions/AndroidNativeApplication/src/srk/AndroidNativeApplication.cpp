#include "AndroidNativeApplication.h"
#include "srk/events/EventDispatcher.h"

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
        _ins = new AndroidNativeApplication(activity, looper, shared);
    }

    void AndroidNativeApplication::setState(State state) {
        std::scoped_lock lock(_mutex);

        if (_state != state) {
            _state = state;
            _eventDispatcher->dispatchEvent(this, Event::STATE_CHANGED, &_state);
        }
    }

    void AndroidNativeApplication::setWindow(ANativeWindow* window) {
        std::scoped_lock lock(_mutex);

        if (_window != window) {
            _window = window;
            _eventDispatcher->dispatchEvent(this, Event::WINDOW_CHANGED, _window);
        }
    }

    void AndroidNativeApplication::setWindowSize(Vec2ui32 size) {
        if (_windowSize != size) {
            _windowSize = size;
            _eventDispatcher->dispatchEvent(this, Event::WINDOW_RESIZE, &_windowSize);
        }
    }

    void AndroidNativeApplication::setInputQueue(AInputQueue* queue) {
        std::scoped_lock lock(_mutex);

        if (_inputQueue != queue) {
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
        std::scoped_lock lock(_mutex);

        if (_hasFocus != hasFocus) {
            _hasFocus = hasFocus;
            _eventDispatcher->dispatchEvent(this, Event::FOCUS_CHANGED, &_hasFocus);
        }
    }

    //void AndroidNativeApplication::onWindowRedrawNeeded(ANativeWindow* window) { }
    //void AndroidNativeApplication::onContentRectChanged(const ARect* rect) { }
    //void AndroidNativeApplication::onConfigurationChanged() { }

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
        _getApp(activity)->setWindowSize(Vec2ui32(ANativeWindow_getWidth(window), ANativeWindow_getHeight(window)));
    }

    void AndroidNativeApplication::_onNativeWindowRedrawNeeded(ANativeActivity* activity, ANativeWindow* window) {
        //_getApp(activity)->onWindowRedrawNeeded(window);
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
        //_getApp(activity)->onContentRectChanged(rect);
    }

    void AndroidNativeApplication::_onConfigurationChanged(ANativeActivity* activity) {
        //_getApp(activity)->onConfigurationChanged();
    }

    void AndroidNativeApplication::_onLowMemory(ANativeActivity* activity) {
        _getApp(activity)->lowMemory();
    }
}