#include "AndroidApp.h"

#if SRK_OS == SRK_OS_ANDROID
#include "srk/Printer.h"
#include "srk/events/EventDispatcher.h"
#include "srk/modules/windows/WindowModule.h"

namespace srk::extensions {
    AndroidNativeAccessor::AndroidNativeAccessor(ANativeActivity* activity, bool shared) :
        _activity(activity),
        _window(nullptr),
        _inputQueue(nullptr),
        _hasFocus(false),
        _state(State::UNKNOWN),
        _eventDispatcher(new events::EventDispatcher<Event>()) {
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

    void AndroidNativeAccessor::setState(State state) {
        using namespace std::string_view_literals;

        std::scoped_lock lock(_mutex);

        if (_state != state) {
            _state = state;
            printaln(L"state changed "sv, (uint8_t)_state);
            _eventDispatcher->dispatchEvent(this, Event::STATE_CHANGED, &_state);
        }
    }

    void AndroidNativeAccessor::setWindow(ANativeWindow* window) {
        using namespace std::string_view_literals;

        std::scoped_lock lock(_mutex);

        if (_window != window) {
            _eventDispatcher->dispatchEvent(this, Event::WINDOW_CHANGING, _window);
            _window = window;
            printaln(L"window changed "sv, _window);
            _eventDispatcher->dispatchEvent(this, Event::WINDOW_CHANGED, _window);
        }
    }

    void AndroidNativeAccessor::setInputQueue(AInputQueue* queue) {
        std::scoped_lock lock(_mutex);

        if (_inputQueue != queue) {
            _eventDispatcher->dispatchEvent(this, Event::INPUT_QUEUE_CHANGING, _inputQueue);
            _inputQueue = queue;
            _eventDispatcher->dispatchEvent(this, Event::INPUT_QUEUE_CHANGED, _inputQueue);
        }
    }

    void* AndroidNativeAccessor::onSaveInstanceState(size_t* outSize) {
        return nullptr;
    }

    void AndroidNativeAccessor::onDestroy() {

    }

    void AndroidNativeAccessor::setFocus(bool hasFocus) {
        using namespace std::string_view_literals;

        std::scoped_lock lock(_mutex);

        if (_hasFocus != hasFocus) {
            _hasFocus = hasFocus;
            printaln(L"focus changed "sv, _hasFocus);
            _eventDispatcher->dispatchEvent(this, Event::FOCUS_CHANGED, &_hasFocus);
        }
    }

    void AndroidNativeAccessor::onWindowResized(ANativeWindow* window) {

    }

    void AndroidNativeAccessor::onWindowRedrawNeeded(ANativeWindow* window) {

    }

    void AndroidNativeAccessor::onContentRectChanged(const ARect* rect) {

    }

    void AndroidNativeAccessor::onConfigurationChanged() {

    }

    void AndroidNativeAccessor::lowMemory() {
        std::scoped_lock lock(_mutex);

        _eventDispatcher->dispatchEvent(this, Event::LOW_MEMORY);
    }

    void AndroidNativeAccessor::_onStart(ANativeActivity* activity) {
        ((AndroidNativeAccessor*)activity->instance)->setState(AndroidNativeAccessor::State::STARTED);
    }

    void AndroidNativeAccessor::_onResume(ANativeActivity* activity) {
        ((AndroidNativeAccessor*)activity->instance)->setState(AndroidNativeAccessor::State::RESUMED);
    }

    void* AndroidNativeAccessor::_onSaveInstanceState(ANativeActivity* activity, size_t* outSize) {
        return ((AndroidNativeAccessor*)activity->instance)->onSaveInstanceState(outSize);
    }

    void AndroidNativeAccessor::_onPause(ANativeActivity* activity) {
        ((AndroidNativeAccessor*)activity->instance)->setState(AndroidNativeAccessor::State::PAUSED);
    }

    void AndroidNativeAccessor::_onStop(ANativeActivity* activity) {
        ((AndroidNativeAccessor*)activity->instance)->setState(AndroidNativeAccessor::State::STOPPED);
    }

    void AndroidNativeAccessor::_onDestroy(ANativeActivity* activity) {
        ((AndroidNativeAccessor*)activity->instance)->onDestroy();
    }

    void AndroidNativeAccessor::_onWindowFocusChanged(ANativeActivity* activity, int hasFocus) {
        ((AndroidNativeAccessor*)activity->instance)->setFocus(hasFocus != 0);
    }

    void AndroidNativeAccessor::_onNativeWindowCreated(ANativeActivity* activity, ANativeWindow* window) {
        ((AndroidNativeAccessor*)activity->instance)->setWindow(window);
    }

    void AndroidNativeAccessor::_onNativeWindowResized(ANativeActivity* activity, ANativeWindow* window) {
        ((AndroidNativeAccessor*)activity->instance)->onWindowResized(window);
    }

    void AndroidNativeAccessor::_onNativeWindowRedrawNeeded(ANativeActivity* activity, ANativeWindow* window) {
        ((AndroidNativeAccessor*)activity->instance)->onWindowRedrawNeeded(window);
    }

    void AndroidNativeAccessor::_onNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow* window) {
        ((AndroidNativeAccessor*)activity->instance)->setWindow(nullptr);
    }

    void AndroidNativeAccessor::_onInputQueueCreated(ANativeActivity* activity, AInputQueue* queue) {
        ((AndroidNativeAccessor*)activity->instance)->setInputQueue(queue);
    }

    void AndroidNativeAccessor::_onInputQueueDestroyed(ANativeActivity* activity, AInputQueue* queue) {
        ((AndroidNativeAccessor*)activity->instance)->setInputQueue(nullptr);
    }

    void AndroidNativeAccessor::_onContentRectChanged(ANativeActivity* activity, const ARect* rect) {
        ((AndroidNativeAccessor*)activity->instance)->onContentRectChanged(rect);
    }

    void AndroidNativeAccessor::_onConfigurationChanged(ANativeActivity* activity) {
        ((AndroidNativeAccessor*)activity->instance)->onConfigurationChanged();
    }

    void AndroidNativeAccessor::_onLowMemory(ANativeActivity* activity) {
        ((AndroidNativeAccessor*)activity->instance)->lowMemory();
    }
}

AndroidApp::AndroidApp(ANativeActivity* activity) :
    _activity(activity),
    _window(nullptr),
    _eventDispatcher(new events::EventDispatcher<srk::modules::windows::WindowEvent>()) {
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

    using namespace std::string_view_literals;

    printaln(L"================================="sv);
    printaln(L"ANativeActivity_onCreate"sv);
    printaln(L"internalDataPath : "sv, std::string_view(_activity->internalDataPath));
    printaln(L"externalDataPath : "sv, std::string_view(_activity->externalDataPath));
    printaln(L"appPath"sv, getAppPath());

    printaln(L"for"sv);
    for (auto& itr : std::filesystem::directory_iterator(std::filesystem::path("/data/data/com.shuriken.test"))) {
        printaln(L"sub : "sv, itr.path().wstring());
    }
}

std::filesystem::path AndroidApp::getAppPath() {
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

void AndroidApp::setWindow(ANativeWindow* window) {
    _window = window;
}

void AndroidApp::focusChanged(bool hasFocus) {
    _eventDispatcher->dispatchEvent(_activity, hasFocus ? srk::modules::windows::WindowEvent::FOCUS_IN : srk::modules::windows::WindowEvent::FOCUS_OUT);
}

void AndroidApp::_onStart(ANativeActivity* activity) {

}

void AndroidApp::_onResume(ANativeActivity* activity) {

}

void* AndroidApp::_onSaveInstanceState(ANativeActivity* activity, size_t* outSize) {
    return nullptr;
}

void AndroidApp::_onPause(ANativeActivity* activity) {

}

void AndroidApp::_onStop(ANativeActivity* activity) {

}

void AndroidApp::_onDestroy(ANativeActivity* activity) {

}

void AndroidApp::_onWindowFocusChanged(ANativeActivity* activity, int hasFocus) {
    _getApp(activity)->focusChanged(hasFocus);
}

void AndroidApp::_onNativeWindowCreated(ANativeActivity* activity, ANativeWindow* window) {
    using namespace std::string_view_literals;

    printaln(L"onNativeWindowCreated"sv);
}

void AndroidApp::_onNativeWindowResized(ANativeActivity* activity, ANativeWindow* window) {

}

void AndroidApp::_onNativeWindowRedrawNeeded(ANativeActivity* activity, ANativeWindow* window) {

}

void AndroidApp::_onNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow* window) {
    using namespace std::string_view_literals;

    printaln(L"onNativeWindowDestroyed"sv);
}

void AndroidApp::_onInputQueueCreated(ANativeActivity* activity, AInputQueue* queue) {

}

void AndroidApp::_onInputQueueDestroyed(ANativeActivity* activity, AInputQueue* queue) {

}

void AndroidApp::_onContentRectChanged(ANativeActivity* activity, const ARect* rect) {

}

void AndroidApp::_onConfigurationChanged(ANativeActivity* activity) {

}

void AndroidApp::_onLowMemory(ANativeActivity* activity) {

}
#endif