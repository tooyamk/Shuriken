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

		virtual bool AE_CALL isEnabled() const override;
		virtual void AE_CALL setEnabled(bool isEnabled) override;
		virtual void AE_CALL pollEvents() override;
		virtual InputModule::CREATE_PARAMS::EVENT_DISPATCHER* AE_CALL getEventDispatcher() const override;

	private:
		class KeyInfo {
		public:
			ui8 code;
			ui8 state;
			i64 timestamp;
		};


		bool _isEnabled;

		Application* _app;
		const InputModule::CREATE_PARAMS::EVENT_DISPATCHER_ALLOCATOR& _eventDispatcherAllocator;
		InputModule::CREATE_PARAMS::EVENT_DISPATCHER* _eventDispatcher;

		using Listener = events::EventListener<ApplicationEvent, Keyboard>;

		Listener _downListener;
		Listener _upListener;

		void _downHandler(events::Event<ApplicationEvent>& e);
		void _upHandler(events::Event<ApplicationEvent>& e);

		std::vector<KeyInfo> _keys;
	};
}


#ifdef AE_MODULE_EXPORTS
extern "C" AE_MODULE_DLL_EXPORT aurora::modules::InputModule* AE_CREATE_MODULE_FN_NAME(aurora::modules::InputModule::CREATE_PARAMS_PTR params) {
	if (!params || !params->application || !params->eventDispatcherAllocator) return nullptr;
	return new aurora::modules::input_win_kb::Keyboard(*params);
}
#endif