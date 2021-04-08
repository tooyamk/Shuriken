#pragma once

#include "Input.h"

namespace aurora::modules::inputs::hid_input {
	template<size_t BufferSize>
	class DeviceBase : public IInputDevice {
	public:
		DeviceBase(Input& input, const DeviceInfo& info, extensions::HIDDevice& hid) : _input(input),
			_info(info),
			_hid(&hid) {
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

			StateBuffer state;
			auto rst = HID::read(*_hid, state, sizeof(StateBuffer), 0);
			if (!HID::isReadSuccess(rst)) return;

			if (!dispatchEvent) {
				std::scoped_lock lock(_mutex);

				memcpy(_state, state, rst);

				return;
			}
		}

		virtual void AE_CALL setDeadZone(uint32_t keyCode, float32_t deadZone) override {}
		virtual void AE_CALL setVibration(float32_t left, float32_t right) override {}

	protected:
		using StateBuffer = uint8_t[BufferSize];

		IntrusivePtr<Input> _input;
		events::EventDispatcher<DeviceEvent> _eventDispatcher;
		DeviceInfo _info;
		extensions::HIDDevice* _hid;

		mutable std::shared_mutex _mutex;
		StateBuffer _state;

		virtual void AE_CALL _parse(StateBuffer state, size_t size) = 0;
	};
}