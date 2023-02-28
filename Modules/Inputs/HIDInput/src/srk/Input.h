#pragma once

#include "Base.h"
#include "srk/events/EventDispatcher.h"
#include "srk/modules/windows/WindowModule.h"
#include <shared_mutex>

namespace srk::modules::inputs::hid_input {
	class SRK_MODULE_DLL Input : public IInputModule {
	public:
		Input(Ref* loader, windows::IWindow* win, DeviceType filter);
		virtual ~Input();

		void operator delete(Input* p, std::destroying_delete_t) {
			auto l = p->_loader;
			p->~Input();
			::operator delete(p);
		}

		virtual IntrusivePtr<events::IEventDispatcher<ModuleEvent>>SRK_CALL getEventDispatcher() override;
		virtual void SRK_CALL poll() override;
		virtual IntrusivePtr<IInputDevice> SRK_CALL createDevice(const DeviceGUID& guid) override;

	private:
		IntrusivePtr<Ref> _loader;
		IntrusivePtr<windows::IWindow> _win;
		DeviceType _filter;
		IntrusivePtr<events::IEventDispatcher<ModuleEvent>> _eventDispatcher;

		std::shared_mutex _mutex;
		std::vector<InternalDeviceInfo> _devices;

		inline bool SRK_CALL _hasDevice(const DeviceInfo& info, const std::vector<InternalDeviceInfo>& devices) const {
			for (auto& di : devices) {
				if (info.guid == di.guid) return true;
			}

			return false;
		}
	};
}