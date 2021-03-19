#pragma once

#include "Base.h"
#include "aurora/events/EventDispatcher.h"

namespace aurora::modules::inputs::generic_input {
	class AE_MODULE_DLL Input : public IInputModule {
	public:
		Input(Ref* loader, IApplication* app);
		virtual ~Input();

		void operator delete(Input* p, std::destroying_delete_t) {
			auto l = p->_loader;
			p->~Input();
			::operator delete(p);
		}

		virtual events::IEventDispatcher<ModuleEvent>& AE_CALL getEventDispatcher() override;
		virtual void AE_CALL poll() override;
		virtual IInputDevice* AE_CALL createDevice(const DeviceGUID& guid) override;

	private:
		IntrusivePtr<Ref> _loader;
		IntrusivePtr<IApplication> _app;

		events::EventDispatcher<ModuleEvent> _eventDispatcher;
	};
}