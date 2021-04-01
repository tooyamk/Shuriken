#pragma once

#include "Base.h"
#include "aurora/events/EventDispatcher.h"

namespace aurora::modules::inputs::direct_input {
	class AE_MODULE_DLL Input : public IInputModule {
	public:
		Input(Ref* loader, IApplication* app, bool ignoreXInputDevices);
		virtual ~Input();

		void operator delete(Input* p, std::destroying_delete_t) {
			auto l = p->_loader;
			p->~Input();
			::operator delete(p);
		}

		virtual events::IEventDispatcher<ModuleEvent>& AE_CALL getEventDispatcher() override;
		virtual void AE_CALL poll() override;
		virtual IntrusivePtr<IInputDevice> AE_CALL createDevice(const DeviceGUID& guid) override;

		HWND AE_CALL getHWND() const;

		static bool AE_CALL isXInputDevice(const ::GUID& guidProduct);

	private:
		struct EnumDevicesData {
			bool ignoreXInputDevices;
			std::vector<DeviceInfo>* devices;
		};


		IntrusivePtr<Ref> _loader;
		IntrusivePtr<IApplication> _app;
		events::EventDispatcher<ModuleEvent> _eventDispatcher;

		std::shared_mutex _mutex;
		std::vector<DeviceInfo> _devices;

		bool _ignoreXInputDevices;
		LPDIRECTINPUT8 _di;

		static BOOL CALLBACK _enumDevicesCallback(const DIDEVICEINSTANCE* pdidInstance, LPVOID pContext);

		inline bool AE_CALL _hasDevice(const DeviceInfo& info, const std::vector<DeviceInfo>& devices) const {
			for (auto& di : devices) {
				if (info.guid == di.guid) return true;
			}

			return false;
		}
	};
}