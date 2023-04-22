#pragma once

#include "Base.h"

namespace srk::modules::inputs::x11 {
	class SRK_MODULE_DLL Input : public IInputModule {
	public:
		Input(Ref* loader, const CreateInputModuleDescriptor& desc);
		virtual ~Input();

		void SRK_CALL operator delete(Input* p, std::destroying_delete_t) {
			auto l = p->_loader;
			p->~Input();
			::operator delete(p);
		}

		virtual IntrusivePtr<events::IEventDispatcher<ModuleEvent>> SRK_CALL getEventDispatcher() override;
		virtual void SRK_CALL poll() override;
		virtual IntrusivePtr<IInputDevice> SRK_CALL createDevice(const DeviceGUID& guid) override;

	private:
		IntrusivePtr<Ref> _loader;
		IntrusivePtr<windows::IWindow> _win;
		IntrusivePtr<events::IEventDispatcher<ModuleEvent>> _eventDispatcher;

		std::vector<DeviceInfo> _devices;
		std::atomic_bool _firstPoll;

		inline static bool SRK_CALL _hasDevice(const DeviceInfo& info, const std::vector<DeviceInfo>& devices) {
			for (auto& di : devices) {
				if (info.guid == di.guid) return true;
			}

			return false;
		}
	};
}