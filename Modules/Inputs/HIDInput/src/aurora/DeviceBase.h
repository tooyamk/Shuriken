#pragma once

#include "Base.h"
#include "aurora/events/EventDispatcher.h"

//#include <hidsdi.h>
//#include <SetupAPI.h>

namespace aurora::modules::inputs::hid_input {
	class Input;

	class AE_MODULE_DLL DeviceBase : public IInputDevice {
	public:
		DeviceBase(Input& input, const DeviceInfo& info);
		virtual ~DeviceBase();

		virtual events::IEventDispatcher<DeviceEvent>& AE_CALL getEventDispatcher() override;
		virtual const DeviceInfo& AE_CALL getInfo() const override;
		virtual void AE_CALL setDeadZone(uint32_t keyCode, float32_t deadZone) override {}
		virtual void AE_CALL setVibration(float32_t left, float32_t right) override {}

		bool AE_CALL open();

	protected:
		IntrusivePtr<Input> _input;
		events::EventDispatcher<DeviceEvent> _eventDispatcher;
		DeviceInfo _info;

		//HANDLE _handle;
		//BYTE* _inputBuffer;
		//USHORT _inputBufferLength;
		//OVERLAPPED _oRead;

		//bool _isReadPending;
		//DWORD _receivedLength;

		//void AE_CALL _read();

		virtual void AE_CALL _parse() = 0;
	};
}