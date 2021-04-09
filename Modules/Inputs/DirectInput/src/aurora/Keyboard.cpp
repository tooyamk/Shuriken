#include "Keyboard.h"
#include "Input.h"

namespace aurora::modules::inputs::direct_input {
	Keyboard::Keyboard(Input& input, LPDIRECTINPUTDEVICE8 dev, const InternalDeviceInfo& info) : DeviceBase(input, dev, info) {
		_dev->SetDataFormat(&c_dfDIKeyboard);
		_dev->SetCooperativeLevel(_input->getHWND(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
		memset(_state, 0, sizeof(StateBuffer));
	}

	Key::CountType Keyboard::getKeyState(Key::CodeType keyCode, Key::ValueType* data, Key::CountType count) const {
		if (data && count && keyCode < sizeof(StateBuffer)) {
			std::shared_lock lock(_mutex);

			switch (keyCode) {
			case VK_SHIFT:
				data[0] = (_state[VK_SK[VK_LSHIFT]] & 0x80) || (_state[VK_SK[VK_RSHIFT]] & 0x80) ? Math::ONE<Key::ValueType> : Math::ZERO<Key::ValueType>;
				return 1;
			case VK_CONTROL:
				data[0] = (_state[VK_SK[VK_LCONTROL]] & 0x80) || (_state[VK_SK[VK_RCONTROL]] & 0x80) ? Math::ONE<Key::ValueType> : Math::ZERO<Key::ValueType>;
				return 1;
			case VK_MENU:
				data[0] = (_state[VK_SK[VK_LMENU]] & 0x80) || (_state[VK_SK[VK_RMENU]] & 0x80) ? Math::ONE<Key::ValueType> : Math::ZERO<Key::ValueType>;
				return 1;
			default:
			{
				if (auto key = VK_SK[keyCode]; key) {
					data[0] = _state[key] & 0x80 ? Math::ONE<Key::ValueType> : Math::ZERO<Key::ValueType>;

					return 1;
				}

				break;
			}
			}
		}

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
			Key::ValueType value = (state[key] & 0x80) > 0 ? Math::ONE<Key::ValueType> : Math::ZERO<Key::ValueType>;

			Key k = { SK_VK[key], 1, &value };
			_eventDispatcher.dispatchEvent(this, value > Math::ZERO<Key::ValueType> ? DeviceEvent::DOWN : DeviceEvent::UP, &k);
		}
	}
}