#pragma once

#include "Base.h"
#include "aurora/events/EventDispatcher.h"

namespace aurora::modules::inputs::hid_input {
	class Input;

	class AE_MODULE_DLL DeviceBase : public IInputDevice {
	public:
		DeviceBase(Input& input, const DeviceInfo& info, extensions::HIDDevice& hid);
		virtual ~DeviceBase();

		virtual events::IEventDispatcher<DeviceEvent>& AE_CALL getEventDispatcher() override;
		virtual const DeviceInfo& AE_CALL getInfo() const override;
		virtual void AE_CALL poll(bool dispatchEvent) override;
		virtual void AE_CALL setDeadZone(uint32_t keyCode, float32_t deadZone) override {}
		virtual void AE_CALL setVibration(float32_t left, float32_t right) override {}

	protected:
		IntrusivePtr<Input> _input;
		events::EventDispatcher<DeviceEvent> _eventDispatcher;
		DeviceInfo _info;
		extensions::HIDDevice* _hid;

		mutable std::shared_mutex _mutex;
		uint8_t* _state;
		size_t _stateSize;

		void AE_CALL _init(uint8_t* state, size_t size);
	};
}