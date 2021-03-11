#include "Keyboard.h"
#include "Input.h"

namespace aurora::modules::inputs::direct_input {
	Keyboard::Keyboard(Input& input, LPDIRECTINPUTDEVICE8 dev, const DeviceInfo& info) : DeviceBase(input, dev, info) {
		_dev->SetDataFormat(&c_dfDIKeyboard);
		_dev->SetCooperativeLevel(_input->getHWND(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
		memset(_state, 0, sizeof(StateBuffer));
	}

	uint32_t Keyboard::getKeyState (uint32_t keyCode, float32_t* data, uint32_t count) const {
		if (data && count && keyCode < 256) {
			switch (keyCode) {
			case VK_SHIFT:
				data[0] = (_state[VK_SK[VK_LSHIFT]] & 0x80) || (_state[VK_SK[VK_RSHIFT]] & 0x80) ? 1.f : 0.f;
				return 1;
			case VK_CONTROL:
				data[0] = (_state[VK_SK[VK_LCONTROL]] & 0x80) || (_state[VK_SK[VK_RCONTROL]] & 0x80) ? 1.f : 0.f;
				return 1;
			case VK_MENU:
				data[0] = (_state[VK_SK[VK_LMENU]] & 0x80) || (_state[VK_SK[VK_RMENU]] & 0x80) ? 1.f : 0.f;
				return 1;
			default:
			{
				auto key = VK_SK[keyCode];
				if (key) {
					data[0] = _state[key] & 0x80 ? 1.f : 0.f;

					return 1;
				} else {
					return 0;
				}
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

		if (!dispatchEvent) {
			_dev->GetDeviceState(sizeof(DIJOYSTATE2), _state);
			return;
		}

		StateBuffer state;
		hr = _dev->GetDeviceState(sizeof(StateBuffer), state);
		if (SUCCEEDED(hr)) {
			StateBuffer changedBtns;
			uint16_t len = 0;
			for (uint16_t i = 0; i < sizeof(StateBuffer); ++i) {
				if (_state[i] != state[i]) {
					_state[i] = state[i];
					changedBtns[len++] = uint8_t(i);
				}
			}

			if (len > 0) {
				//MapVirtualKeyEx(DIK_RCONTROL, MAPVK_VSC_TO_VK_EX, GetKeyboardLayout(0));
				//auto layout = GetKeyboardLayout(0);
				for (uint16_t i = 0; i < len; ++i) {
					uint8_t key = changedBtns[i];
					float32_t value = (state[key] & 0x80) > 0 ? 1.f : 0.f;

					Key k = { SK_VK[key], 1, &value };
					_eventDispatcher.dispatchEvent(this, value > 0.f ? DeviceEvent::DOWN : DeviceEvent::UP, &k);
				}
			}
		}
	}
}