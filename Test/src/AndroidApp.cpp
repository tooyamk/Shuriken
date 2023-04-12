#include "AndroidApp.h"

#if SRK_OS == SRK_OS_ANDROID
#include "srk/Printer.h"
#include "srk/events/EventDispatcher.h"
#include "srk/modules/windows/WindowModule.h"

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