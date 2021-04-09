#pragma once

#include "Input.h"

namespace aurora::modules::inputs::hid_input {
	template<size_t StateBufferSize, size_t ReadBufferSize>
	class DeviceBase : public IInputDevice {
	public:
		DeviceBase(Input& input, const DeviceInfo& info, extensions::HIDDevice& hid) : _input(input),
			_info(info),
			_hid(&hid) {
			memset(_state, 0, sizeof(StateBuffer));
		}

		virtual ~DeviceBase() {
			using namespace aurora::extensions;

			HID::close(*_hid);
		}

		virtual events::IEventDispatcher<DeviceEvent>& AE_CALL getEventDispatcher() override {
			return _eventDispatcher;
		}

		virtual const DeviceInfo& AE_CALL getInfo() const override {
			return _info;
		}

		virtual void AE_CALL poll(bool dispatchEvent) override {
			using namespace aurora::extensions;

			ReadBuffer state;
			auto rst = HID::read(*_hid, state, sizeof(ReadBuffer), 0);
			if (!HID::isReadSuccess(rst)) return;

			if (rst) _parse(dispatchEvent, state, rst);
		}

		virtual void AE_CALL setDeadZone(uint32_t keyCode, float32_t deadZone) override {}
		virtual void AE_CALL setVibration(float32_t left, float32_t right) override {}

	protected:
		using StateBuffer = uint8_t[StateBufferSize];
		using ReadBuffer = uint8_t[ReadBufferSize];

		IntrusivePtr<Input> _input;
		events::EventDispatcher<DeviceEvent> _eventDispatcher;
		DeviceInfo _info;
		extensions::HIDDevice* _hid;

		mutable std::shared_mutex _mutex;
		StateBuffer _state;

		virtual void AE_CALL _parse(bool dispatchEvent, ReadBuffer& readBuffer, size_t readBufferSize) = 0;
	};
}