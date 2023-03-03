#include "Keyboard.h"
#include "Input.h"

namespace srk::modules::inputs::direct_input {
	Keyboard::Keyboard(Input& input, LPDIRECTINPUTDEVICE8 dev, const DeviceInfo& info) : DeviceBase(input, dev, info) {
		_dev->SetDataFormat(&c_dfDIKeyboard);
		_dev->SetCooperativeLevel(_input->getHWND(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
		memset(_state, 0, sizeof(StateBuffer));
	}

	DeviceState::CountType Keyboard::getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const {
		switch (type) {
		case DeviceStateType::KEY:
		{
			if (values && count && code < sizeof(StateBuffer)) {
				std::shared_lock lock(_mutex);

				switch (code) {
				case VK_SHIFT:
					((DeviceStateValue*)values)[0] = (_state[VK_SK[VK_LSHIFT]] & 0x80) || (_state[VK_SK[VK_RSHIFT]] & 0x80) ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;
					return 1;
				case VK_CONTROL:
					((DeviceStateValue*)values)[0] = (_state[VK_SK[VK_LCONTROL]] & 0x80) || (_state[VK_SK[VK_RCONTROL]] & 0x80) ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;
					return 1;
				case VK_MENU:
					((DeviceStateValue*)values)[0] = (_state[VK_SK[VK_LMENU]] & 0x80) || (_state[VK_SK[VK_RMENU]] & 0x80) ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;
					return 1;
				default:
				{
					if (auto key = VK_SK[code]; key) {
						((DeviceStateValue*)values)[0] = _state[key] & 0x80 ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;

						return 1;
					}

					return 0;
				}
				}
			}

			return 0;
		}
		default:
			return 0;
		}
	}

	DeviceState::CountType Keyboard::setState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count) {
		return 0;
	}

	void Keyboard::poll(bool dispatchEvent) {
		HRESULT hr = _dev->Poll();
		if (hr == DIERR_NOTACQUIRED || hr == DIERR_INPUTLOST) {
			if (FAILED(_dev->Acquire())) return;
			if (FAILED(_dev->Poll())) return;
		}

		StateBuffer state;
		hr = _dev->GetDeviceState(sizeof(StateBuffer), state);
		if (!SUCCEEDED(hr)) return;

		if (!dispatchEvent) {
			std::scoped_lock lock(_mutex);

			memcpy(_state, state, sizeof(StateBuffer));

			return;
		}

		StateBuffer changedBtns;
		uint16_t len = 0;

		{
			std::scoped_lock lock(_mutex);

			for (uint16_t i = 0; i < sizeof(StateBuffer); ++i) {
				if (_state[i] != state[i]) {
					_state[i] = state[i];
					changedBtns[len++] = uint8_t(i);
				}
			}
		}

		for (uint16_t i = 0; i < len; ++i) {
			auto key = changedBtns[i];
			DeviceStateValue value = (state[key] & 0x80) > 0 ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;

			DeviceState k = { SK_VK[key], 1, &value };
			_eventDispatcher->dispatchEvent(this, value > Math::ZERO<DeviceStateValue> ? DeviceEvent::DOWN : DeviceEvent::UP, &k);
		}
	}
}