#pragma once

#include "Base.h"
#include "srk/events/EventDispatcher.h"

namespace srk::modules::inputs::xinput {
	class SRK_MODULE_DLL Input : public IInputModule {
	public:
		Input(Ref* loader, DeviceType filter);
		virtual ~Input();

		void operator delete(Input* p, std::destroying_delete_t) {
			auto l = p->_loader;
			p->~Input();
			::operator delete(p);
		}

		virtual IntrusivePtr<events::IEventDispatcher<ModuleEvent>> SRK_CALL getEventDispatcher() override;
		virtual void SRK_CALL poll() override;
		virtual IntrusivePtr<IInputDevice> SRK_CALL createDevice(const DeviceGUID& guid) override;

	private:
		IntrusivePtr<Ref> _loader;
		DeviceType _filter;
		IntrusivePtr<events::IEventDispatcher<ModuleEvent>> _eventDispatcher;

		std::shared_mutex _mutex;
		std::vector<DeviceInfo> _devices;

		inline bool SRK_CALL _hasDevice(const DeviceInfo& info, const std::vector<DeviceInfo>& devices) const {
			for (auto& di : devices) {
				if (info.guid == di.guid) return true;
			}

			return false;
		}
	};
}