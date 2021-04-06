#pragma once

#include "Base.h"
#include "aurora/events/EventDispatcher.h"

namespace aurora::modules::inputs::xinput {
	class AE_MODULE_DLL Input : public IInputModule {
	public:
		Input(Ref* loader, DeviceType filter);
		virtual ~Input();

		void operator delete(Input* p, std::destroying_delete_t) {
			auto l = p->_loader;
			p->~Input();
			::operator delete(p);
		}

		virtual events::IEventDispatcher<ModuleEvent>& AE_CALL getEventDispatcher() override;
		virtual void AE_CALL poll() override;
		virtual IntrusivePtr<IInputDevice> AE_CALL createDevice(const DeviceGUID& guid) override;

	private:
		IntrusivePtr<Ref> _loader;
		DeviceType _filter;
		events::EventDispatcher<ModuleEvent> _eventDispatcher;

		std::shared_mutex _mutex;
		std::vector<DeviceInfo> _devices;

		inline bool AE_CALL _hasDevice(const DeviceInfo& info, const std::vector<DeviceInfo>& devices) const {
			for (auto& di : devices) {
				if (info.guid == di.guid) return true;
			}

			return false;
		}
	};
}