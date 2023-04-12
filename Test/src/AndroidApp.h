#pragma once

#include "srk/Global.h"

#if SRK_OS == SRK_OS_ANDROID
#include "srk/Intrusive.h"
#include <android/native_activity.h>
#include <filesystem>

using namespace srk;

namespace srk::events {
	template<typename EvtType> class IEventDispatcher;
}

namespace srk::modules::windows {
    enum class WindowEvent : uint8_t;
}

class AndroidApp {
public:
    AndroidApp(ANativeActivity* activity);

    std::filesystem::path SRK_CALL getAppPath();
    inline IntrusivePtr<events::IEventDispatcher<srk::modules::windows::WindowEvent>> SRK_CALL getEventDispatcher() const {
        return _eventDispatcher;
    }

    void SRK_CALL setWindow(ANativeWindow* window);

    void SRK_CALL focusChanged(bool hasFocus);

private:
    ANativeActivity* _activity;
    ANativeWindow* _window;
    IntrusivePtr<events::IEventDispatcher<srk::modules::windows::WindowEvent>> _eventDispatcher;

    static void _onStart(ANativeActivity* activity);
    static void _onResume(ANativeActivity* activity);
    static void* _onSaveInstanceState(ANativeActivity* activity, size_t* outSize);
    static void _onPause(ANativeActivity* activity);
    static void _onStop(ANativeActivity* activity);
    static void _onDestroy(ANativeActivity* activity);
    static void _onWindowFocusChanged(ANativeActivity* activity, int hasFocus);
    static void _onNativeWindowCreated(ANativeActivity* activity, ANativeWindow* window);
    static void _onNativeWindowResized(ANativeActivity* activity, ANativeWindow* window);
    static void _onNativeWindowRedrawNeeded(ANativeActivity* activity, ANativeWindow* window);
    static void _onNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow* window);
    static void _onInputQueueCreated(ANativeActivity* activity, AInputQueue* queue);
    static void _onInputQueueDestroyed(ANativeActivity* activity, AInputQueue* queue);
    static void _onContentRectChanged(ANativeActivity* activity, const ARect* rect);
    static void _onConfigurationChanged(ANativeActivity* activity);
    static void _onLowMemory(ANativeActivity* activity);

    static AndroidApp* SRK_CALL _getApp(ANativeActivity* activity) {
        return (AndroidApp*)activity->instance;
    }
};
#endif