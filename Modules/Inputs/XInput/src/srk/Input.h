#pragma once

#include "Base.h"
#include "srk/events/EventDispatcher.h"

namespace srk::modules::inputs::xinput {
	class SRK_MODULE_DLL Input : public IInputModule {
	public:
		Input(Ref* loader, const CreateInputModuleDescriptor& desc);
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
		struct XINPUT_CAPABILITIES_EX {
			XINPUT_CAPABILITIES Capabilities;
			WORD vendorId;
			WORD productId;
			WORD versionNumber;
			WORD unknown1;
			DWORD unknown2;
		};


		using XInputGetCapabilitiesEx = DWORD(_stdcall*)(DWORD a1, DWORD dwUserIndex, DWORD dwFlags, XINPUT_CAPABILITIES_EX* pCapabilities);

		IntrusivePtr<Ref> _loader;
		DeviceType _filters;
		IntrusivePtr<events::IEventDispatcher<ModuleEvent>> _eventDispatcher;

		XInputGetCapabilitiesEx _XInputGetCapabilitiesEx;

		std::shared_mutex _mutex;
		std::vector<DeviceInfo> _devices;

		inline static bool SRK_CALL _hasDevice(const DeviceInfo& info, const std::vector<DeviceInfo>& devices) {
			for (auto& di : devices) {
				if (info.guid == di.guid) return true;
			}

			return false;
		}
	};
}