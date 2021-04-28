#pragma once

#include "Input.h"

namespace aurora::modules::inputs::hid_input {
	template<size_t InputStateBufferSize, size_t InputBufferSize, size_t OutputStateBufferSize>
	class DeviceBase : public IInputDevice {
	public:
		DeviceBase(Input& input, const DeviceInfo& info, extensions::HIDDevice& hid) :
			_input(input),
			_eventDispatcher(new events::EventDispatcher<DeviceEvent>()),
			_info(info),
			_hid(&hid),
			_needOutput(false) {
			memset(_inputState, 0, sizeof(InputStateBuffer));
		}

		virtual ~DeviceBase() {
			using namespace aurora::extensions;

			HID::close(*_hid);
		}

		virtual IntrusivePtr<events::IEventDispatcher<DeviceEvent>> AE_CALL getEventDispatcher() override {
			return _eventDispatcher;
		}

		virtual const DeviceInfo& AE_CALL getInfo() const override {
			return _info;
		}

		virtual void AE_CALL poll(bool dispatchEvent) override {
			using namespace aurora::extensions;

			{
				InputBuffer state;
				auto rst = HID::read(*_hid, state, sizeof(InputBuffer), 0);
				if (!HID::isSuccess(rst)) return;

				if (rst) _doInput(dispatchEvent, state, rst);
			}

			{
				std::scoped_lock lock(_outputStateMutex);

				_needOutput |= _doOutput();
				if (_needOutput && HID::isSuccess(HID::write(*_hid, _outputState, sizeof(OutputStateBuffer), 0))) _needOutput = false;
			}
		}

	protected:
		using DeviceBaseType = DeviceBase<InputStateBufferSize, InputBufferSize, OutputStateBufferSize>;

		using InputStateBuffer = uint8_t[InputStateBufferSize];
		using InputBuffer = uint8_t[InputBufferSize];
		using OutputStateBuffer = uint8_t[OutputStateBufferSize];

		IntrusivePtr<Input> _input;
		IntrusivePtr<events::IEventDispatcher<DeviceEvent>> _eventDispatcher;
		DeviceInfo _info;
		extensions::HIDDevice* _hid;

		InputStateBuffer _inputState;

		mutable std::shared_mutex _outputStateMutex;
		bool _needOutput;
		OutputStateBuffer _outputState;

		virtual void AE_CALL _doInput(bool dispatchEvent, InputBuffer& inputBuffer, size_t inputBufferSize) = 0;
		virtual bool AE_CALL _doOutput() = 0;
	};
}