#pragma once

#include "Base.h"
#include "aurora/events/EventDispatcher.h"
#include <shared_mutex>

namespace aurora::modules::inputs::hid_input {
	class AE_MODULE_DLL Input : public IInputModule {
	public:
		Input(Ref* loader, IApplication* app, DeviceType filter);
		virtual ~Input();

		void operator delete(Input* p, std::destroying_delete_t) {
			auto l = p->_loader;
			p->~Input();
			::operator delete(p);
		}

		virtual IntrusivePtr<events::IEventDispatcher<ModuleEvent>>AE_CALL getEventDispatcher() override;
		virtual void AE_CALL poll() override;
		virtual IntrusivePtr<IInputDevice> AE_CALL createDevice(const DeviceGUID& guid) override;

	private:
		IntrusivePtr<Ref> _loader;
		IntrusivePtr<IApplication> _app;
		DeviceType _filter;
		IntrusivePtr<events::IEventDispatcher<ModuleEvent>> _eventDispatcher;

		std::shared_mutex _mutex;
		std::vector<InternalDeviceInfo> _devices;

		inline bool AE_CALL _hasDevice(const DeviceInfo& info, const std::vector<InternalDeviceInfo>& devices) const {
			for (auto& di : devices) {
				if (info.guid == di.guid) return true;
			}

			return false;
		}
	};
}