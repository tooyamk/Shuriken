#pragma once

#if AE_OS != AE_OS_WIN
#include "Base.h"
#include "aurora/events/EventDispatcher.h"

namespace aurora::modules::inputs::generic_input {
	using namespace std::literals;

	class AE_MODULE_DLL Input : public IInputModule {
	public:
		Input(Ref* loader) : _loader(loader) {}
		virtual ~Input() {}

		void operator delete(Input* p, std::destroying_delete_t) {
			auto l = p->_loader;
			p->~Input();
			::operator delete(p);
		}

		virtual events::IEventDispatcher<ModuleEvent>& AE_CALL getEventDispatcher() override {
			return _eventDispatcher;
		}
		virtual void AE_CALL poll() override {}
		virtual IntrusivePtr<IInputDevice> AE_CALL createDevice(const DeviceGUID& guid) override {
			return nullptr;
		}

	private:
		IntrusivePtr<Ref> _loader;

		events::EventDispatcher<ModuleEvent> _eventDispatcher;
	};
}
#endif