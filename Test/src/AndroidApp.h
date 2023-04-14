/*
adb install -t -d -r "any.apk"
adb.exe shell  am start com.shuriken.test/android.app.NativeActivity

adb uninstall "com.shuriken.test"

adb logcat -b all -c
adb logcat Shuriken:V *:S
*/

#pragma once

#include "srk/Global.h"

#if SRK_OS == SRK_OS_ANDROID
#include "srk/Intrusive.h"
#include <android/looper.h>
#include <android/native_activity.h>
#include <filesystem>

#include <mutex>

using namespace srk;

namespace srk::events {
	template<typename EvtType> class IEventDispatcher;
}

namespace srk::extensions {
    class AndroidNativeApplication {
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
            WINDOW_CHANGING,
            WINDOW_CHANGED,
            INPUT_QUEUE_CHANGING,
            INPUT_QUEUE_CHANGED,
            FOCUS_CHANGED,
            LOW_MEMORY
        };


        enum class EventType : uint8_t {
            START,
            RESUME,
            PAUSE,
            STOP
        };


        struct Event1 {

        };


        ~AndroidNativeApplication();

        static void init(ANativeActivity* activity, ALooper* looper, bool shared);
        static AndroidNativeApplication* SRK_CALL getInstance() {
            return _ins;
        }

        void SRK_CALL setState(State state);
        inline ANativeWindow* SRK_CALL getWindow() const {
            return _window;
        }
        void SRK_CALL setWindow(ANativeWindow* window);
        void SRK_CALL setInputQueue(AInputQueue* queue);
        void* SRK_CALL onSaveInstanceState(size_t* outSize);
        void SRK_CALL onDestroy();
        void SRK_CALL setFocus(bool hasFocus);
        void SRK_CALL onWindowResized(ANativeWindow* window);
        void SRK_CALL onWindowRedrawNeeded(ANativeWindow* window);
        void SRK_CALL onContentRectChanged(const ARect* rect);
        void SRK_CALL onConfigurationChanged();
        void SRK_CALL lowMemory();

        std::filesystem::path SRK_CALL getAppPath();

        inline IntrusivePtr<events::IEventDispatcher<Event>> SRK_CALL getEventDispatcher() const {
            return _eventDispatcher;
        }

        template<typename Fn>
        void SRK_CALL lockDo(void* userData, Fn&& fn) {
            std::scoped_lock lock(_mutex);

            fn(*this, userData);
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

#include "srk/modules/windows/WindowModule.h"

namespace srk::modules::windows::android_native {
    class Window;

    class Manager : public DefaultWindowModule {
	public:
		Manager();
		virtual ~Manager();

		virtual IntrusivePtr<IWindow> SRK_CALL crerate(const CreateWindowDescriptor& desc) override;
		virtual bool SRK_CALL processEvent(const IWindowModule::EventFn& fn) const override;

		inline void SRK_CALL add(void* nativeWindow, IWindow* window) {
			_add(nativeWindow, window);
		}

		inline void SRK_CALL remove(void* nativeWindow) {
			_remove(nativeWindow);
		}

    private:
        Window* _window;
	};

    //===================================

    class Window : public IWindow {
	public:
		Window(Manager& manager);
		virtual ~Window();

		void operator delete(Window* p, std::destroying_delete_t) {
			auto m = p->_manager;
			p->~Window();
			::operator delete(p);
		}

		virtual IntrusivePtr<events::IEventDispatcher<WindowEvent>> SRK_CALL getEventDispatcher() const override;

		bool SRK_CALL create(const CreateWindowDescriptor& desc);

		virtual bool SRK_CALL isClosed() const override;
		virtual void* SRK_CALL getNative(const std::string_view& native) const override;
		virtual bool SRK_CALL isFullScreen() const override;
		virtual void SRK_CALL toggleFullScreen() override;
		virtual Vec4ui32 SRK_CALL getFrameExtents() const override;
		virtual Vec2ui32 SRK_CALL getContentSize() const override;
		virtual void SRK_CALL setContentSize(const Vec2ui32& size) override;
		virtual std::string_view SRK_CALL getTitle() const override;
		virtual void SRK_CALL setTitle(const std::string_view& title) override;
		virtual void SRK_CALL setPosition(const Vec2i32& pos) override;
		virtual void SRK_CALL setCursorVisible(bool visible) override;
		virtual bool SRK_CALL hasFocus() const override;
		virtual void SRK_CALL setFocus() override;
		virtual bool SRK_CALL isMaximized() const override;
		virtual void SRK_CALL setMaximized() override;
		virtual bool SRK_CALL isMinimized() const override;
		virtual void SRK_CALL setMinimized() override;
		virtual void SRK_CALL setRestore() override;
		virtual bool SRK_CALL isVisible() const override;
		virtual void SRK_CALL setVisible(bool b) override;
		virtual void SRK_CALL close() override;
		virtual void SRK_CALL processEvent(void* data) override;

	protected:
		IntrusivePtr<Manager> _manager;
		IntrusivePtr<events::IEventDispatcher<WindowEvent>> _eventDispatcher;

		//platform
		struct {
			bool isCreated = false;
			std::string title;
            void* wnd = nullptr;

			Vec2ui32 sentContentSize;
		} _data;
        
		//void SRK_CALL _sendResizedEvent();
        //static void SRK_CALL _proc(void* target, Msg msg, void* param);
	};
}
#endif