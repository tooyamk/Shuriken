#pragma once

#include "events/EventDispatcher.h"

#include "Base.h"

namespace aurora::modules::win_direct_input {
	class AE_MODULE_DLL DirectInput : public InputModule {
	public:
		DirectInput();
		virtual ~DirectInput();

		virtual events::IEventDispatcher<InputModuleEvent>& AE_CALL getEventDispatcher() override;
		virtual void AE_CALL poll() override;
		virtual InputDevice* AE_CALL createDevice(const InputDeviceGUID& guid) const override;

	private:
		LPDIRECTINPUT8 _di;

		std::vector<InputDeviceInfo> _devices;
		std::vector<InputDeviceInfo> _connectedDevices;
		std::vector<ui32> _keepDevices;
	
		events::EventDispatcher<InputModuleEvent> _eventDispatcher;

		static BOOL CALLBACK _enumDevicesCallback(const DIDEVICEINSTANCE* pdidInstance, LPVOID pContext);
	};
}


#ifdef AE_MODULE_EXPORTS
extern "C" AE_MODULE_DLL_EXPORT aurora::modules::InputModule* AE_CREATE_MODULE_FN_NAME(aurora::modules::InputModule::CREATE_PARAMS_PTR params) {
	return new aurora::modules::win_direct_input::DirectInput();
}
#endif