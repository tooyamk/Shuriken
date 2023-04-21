#pragma once

#include "srk/Intrusive.h"
#include "srk/events/IEventDispatcher.h"
#include "srk/math/Vector.h"
#include <android/looper.h>
#include <android/native_activity.h>
#include <filesystem>
#include <mutex>

#ifdef SRK_EXT_AN_NTV_APP_EXPORTS
#	define SRK_EXT_AN_NTV_APP_DLL SRK_DLL_EXPORT
#else
#	define SRK_EXT_AN_NTV_APP_DLL SRK_DLL_IMPORT
#endif

namespace srk::extensions {
	class SRK_EXT_AN_NTV_APP_DLL AndroidNativeApplication {
    public:
        enum class State : uint8_t {
            UNKNOWN,
            STARTED,
            RESUMED,
            PAUSED,
            STOPPED
        };


        enum class Event : uint8_t {
            STATE_CHANGED,
            WINDOW_CHANGED,
            WINDOW_RESIZE,
            INPUT_QUEUE_CHANGED,
            FOCUS_CHANGED,
            LOW_MEMORY
        };


        ~AndroidNativeApplication();

        static void init(ANativeActivity* activity, ALooper* looper, bool shared);
        static AndroidNativeApplication* SRK_CALL getInstance() {
            return _ins;
        }

        inline ANativeActivity* SRK_CALL getActivity() const {
            return _activity;
        }

        inline State SRK_CALL getState() const {
            return _state;
        }
        void SRK_CALL setState(State state);

        inline ANativeWindow* SRK_CALL getWindow() const {
            return _window;
        }
        void SRK_CALL setWindow(ANativeWindow* window);

        inline const Vec2ui32& SRK_CALL getWindowSize() const {
            return _windowSize;
        }
        void SRK_CALL setWindowSize(Vec2ui32 size);

        void SRK_CALL setInputQueue(AInputQueue* queue);
        void* SRK_CALL onSaveInstanceState(size_t* outSize);
        void SRK_CALL onDestroy();

        inline bool SRK_CALL hasFocus() const {
            return _hasFocus;
        }
        void SRK_CALL setFocus(bool hasFocus);

        //void SRK_CALL onWindowRedrawNeeded(ANativeWindow* window);
        //void SRK_CALL onContentRectChanged(const ARect* rect);
        //void SRK_CALL onConfigurationChanged();
        void SRK_CALL lowMemory();

        std::filesystem::path SRK_CALL getAppPath();

        inline IntrusivePtr<events::IEventDispatcher<Event>> SRK_CALL getEventDispatcher() const {
            return _eventDispatcher;
        }

        template<typename Fn, typename T>
        void SRK_CALL lockDo(T&& userData, Fn&& fn) {
            std::scoped_lock lock(_mutex);

            fn(*this, std::forward<T>(userData));
        }

    private:
        AndroidNativeApplication(ANativeActivity* activity, ALooper* looper, bool shared);

        static AndroidNativeApplication* _ins;
        
        IntrusivePtr<events::IEventDispatcher<Event>> _eventDispatcher;

        std::recursive_mutex _mutex;
        
        ANativeActivity* _activity;
        ANativeWindow* _window;
        AInputQueue* _inputQueue;
        ALooper* _looper;

        bool _hasFocus;
        State _state;
        Vec2ui32 _windowSize;

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

        static AndroidNativeApplication* SRK_CALL _getApp(ANativeActivity* activity) {
            return (AndroidNativeApplication*)activity->instance;
        }
    };
}