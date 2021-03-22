#pragma once

#include "Base.h"
#include "aurora/events/EventDispatcher.h"

namespace aurora::modules::inputs::direct_input {
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

		HWND AE_CALL getHWND() const;

	private:
		IntrusivePtr<Ref> _loader;
		IntrusivePtr<IApplication> _app;

		events::EventDispatcher<ModuleEvent> _eventDispatcher;

		std::vector<DeviceInfo> _devices;
		std::vector<DeviceInfo> _newDevices;
		std::vector<uint32_t> _keepDevices;

		LPDIRECTINPUT8 _di;

		static BOOL CALLBACK _enumDevicesCallback(const DIDEVICEINSTANCE* pdidInstance, LPVOID pContext);
	};
}