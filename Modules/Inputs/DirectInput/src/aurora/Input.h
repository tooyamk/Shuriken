#pragma once

#include "Base.h"
#include "aurora/events/EventDispatcher.h"

namespace aurora::modules::inputs::direct_input {
	class AE_MODULE_DLL Input : public IInputModule {
	public:
		Input(Ref* loader, IApplication* app);
		virtual ~Input();

		virtual events::IEventDispatcher<ModuleEvent>& AE_CALL getEventDispatcher() override;
		virtual void AE_CALL poll() override;
		virtual IInputDevice* AE_CALL createDevice(const DeviceGUID& guid) override;

		HWND AE_CALL getHWND() const;

	public:
#ifdef __cpp_lib_destroying_delete
		void operator delete(Input* p, std::destroying_delete_t) {
			auto l = p->_loader;
			p->~Input();
			::operator delete(p);
		}
	protected:
#else
		virtual ScopeGuard AE_CALL _destruction() const override {
			auto l = _loader;
			return [l]() {};
		}
#endif

	private:
		IntrusivePtr<Ref> _loader;
		IntrusivePtr<IApplication> _app;

		LPDIRECTINPUT8 _di;

		std::vector<DeviceInfo> _devices;
		std::vector<DeviceInfo> _connectedDevices;
		std::vector<uint32_t> _keepDevices;
	
		events::EventDispatcher<ModuleEvent> _eventDispatcher;

		static BOOL CALLBACK _enumDevicesCallback(const DIDEVICEINSTANCE* pdidInstance, LPVOID pContext);
	};
}