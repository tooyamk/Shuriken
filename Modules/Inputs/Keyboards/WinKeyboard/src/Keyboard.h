#pragma once

#include "modules/InputModule.h"
#include "events/IEventDispatcher.h"

namespace aurora {
	enum class ApplicationEvent : ui8;
}

namespace aurora::modules::input_win_kb {
	class AE_MODULE_DLL Keyboard : public InputModule {
	public:
		Keyboard(Keyboard::CREATE_PARAMS_REF params);
		virtual ~Keyboard();

		virtual ui32 AE_CALL getType() const override;

		virtual void AE_CALL setEnabled(bool isEnabled) override;
		virtual void AE_CALL pollEvents() override;
		virtual InputModule::CREATE_PARAMS::EVENT_DISPATCHER* AE_CALL getEventDispatcher() const override;

	private:
		bool _isDispatching;
	
		Application* _app;
		const InputModule::CREATE_PARAMS::EVENT_DISPATCHER_ALLOCATOR& _eventDispatcherAllocator;
		InputModule::CREATE_PARAMS::EVENT_DISPATCHER* _eventDispatcher;

		using Listener = events::EventListener<ApplicationEvent, Keyboard>;

		Listener _downListener;
		Listener _upListener;

		struct Buf {
			i8 data[sizeof(InputEvent) + sizeof(InputKey)];
		};

		void _downHandler(events::Event<ApplicationEvent>& e);
		void _upHandler(events::Event<ApplicationEvent>& e);

		std::vector<Buf> _keys;

		inline InputEvent& _getWritableEventType(Buf& buf) {
			return *(InputEvent*)buf.data;
		}

		inline InputKey& _getWritableKey(Buf& buf) {
			return *(InputKey*)(buf.data + sizeof(InputEvent));
		}

		void _writeKeyInfo(InputEvent type, const MSG& msg, f32 value);
	};
}


#ifdef AE_MODULE_EXPORTS
extern "C" AE_MODULE_DLL_EXPORT aurora::modules::InputModule* AE_CREATE_MODULE_FN_NAME(aurora::modules::InputModule::CREATE_PARAMS_PTR params) {
	if (!params || !params->application || !params->eventDispatcherAllocator) return nullptr;
	return new aurora::modules::input_win_kb::Keyboard(*params);
}
#endif