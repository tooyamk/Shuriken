#pragma once

#include "Base.h"
#include "srk/events/EventDispatcher.h"
#include "srk/modules/windows/IWindowModule.h"

namespace srk::modules::inputs::direct_input {
	class SRK_MODULE_DLL Input : public IInputModule {
	public:
		Input(Ref* loader, windows::IWindow* win, DeviceType filter, bool ignoreXInputDevices);
		virtual ~Input();

		void operator delete(Input* p, std::destroying_delete_t) {
			auto l = p->_loader;
			p->~Input();
			::operator delete(p);
		}

		virtual IntrusivePtr<events::IEventDispatcher<ModuleEvent>> SRK_CALL getEventDispatcher() override;
		virtual void SRK_CALL poll() override;
		virtual IntrusivePtr<IInputDevice> SRK_CALL createDevice(const DeviceGUID& guid) override;

		HWND SRK_CALL getHWND() const;

		static bool SRK_CALL isXInputDevice(const ::GUID& guidProduct);

	private:
		struct EnumDevicesData {
			DeviceType filter;
			bool ignoreXInputDevices;
			std::vector<InternalDeviceInfo>* devices;
		};


		IntrusivePtr<Ref> _loader;
		IntrusivePtr<windows::IWindow> _win;
		DeviceType _filter;
		IntrusivePtr<events::IEventDispatcher<ModuleEvent>> _eventDispatcher;

		std::shared_mutex _mutex;
		std::vector<InternalDeviceInfo> _devices;

		bool _ignoreXInputDevices;
		LPDIRECTINPUT8 _di;

		static BOOL CALLBACK _enumDevicesCallback(const DIDEVICEINSTANCE* pdidInstance, LPVOID pContext);

		inline bool SRK_CALL _hasDevice(const DeviceInfo& info, const std::vector<InternalDeviceInfo>& devices) const {
			for (auto& di : devices) {
				if (info.guid == di.guid) return true;
			}

			return false;
		}
	};
}