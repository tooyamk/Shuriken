#pragma once

#include "srk/lockfree/RingQueue.h"
#include "srk/modules/windows/WindowModule.h"

#include "srk/AndroidNativeApplication.h"
#include <condition_variable>
#include <mutex>

namespace srk::modules::windows::android_native {
	class SRK_MODULE_DLL Manager : public DefaultWindowModule {
	public:
		struct Message {
            extensions::AndroidNativeApplication::Event type;

			union {
                extensions::AndroidNativeApplication::State state;
				bool hasFocus;
				ANativeWindow* window;
			} data;
		};


		Manager(Ref* loader);
		virtual ~Manager();

		virtual IntrusivePtr<IWindow> SRK_CALL crerate(const CreateWindowDescriptor& desc) override;
		virtual bool SRK_CALL processEvent(const IWindowModule::EventFn& fn) const override;

		inline void SRK_CALL add(void* nativeWindow, IWindow* window) {
			_add(nativeWindow, window);
		}

		inline void SRK_CALL remove(void* nativeWindow) {
			_remove(nativeWindow);
		}

        inline ANativeWindow* SRK_CALL getNativeWindow() const {
            return _nativeWindow;
        }

        inline bool SRK_CALL hasFocus() const {
            return _hasFocus;
        }

    private:
		IntrusivePtr<Ref> _loader;

        mutable std::mutex _mutex;
        mutable std::condition_variable _cond;
        mutable ANativeWindow* _nativeWindow;
        mutable ANativeWindow* _processedNativeWindow;

        mutable bool _hasFocus;

        events::EventListener<extensions::AndroidNativeApplication::Event, events::EvtMethod<extensions::AndroidNativeApplication::Event, Manager>> _evtHandler;
        mutable lockfree::RingQueue<Message, lockfree::RingQueueMode::SPSC> _evtQueue;

        void SRK_CALL _recvEvent(events::Event<extensions::AndroidNativeApplication::Event>& e);
	};
}