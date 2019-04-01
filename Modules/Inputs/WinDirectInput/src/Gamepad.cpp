#include "Gamepad.h"
#include "DirectInput.h"

namespace aurora::modules::win_direct_input {
	Gamepad::Gamepad(DirectInput* input, LPDIRECTINPUTDEVICE8 dev, const InputDeviceInfo& info) : DeviceBase(input, dev, info) {
		_dev->SetDataFormat(&c_dfDIJoystick2);
		_dev->SetCooperativeLevel(input->getHWND(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
		memset(&_state, 0, sizeof(DIJOYSTATE2));
	}

	ui32 Gamepad::getKeyState(ui32 keyCode, f32* data, ui32 count) const {
		if (data && count) {
			/*
			ui32 key = MapVirtualKeyEx(keyCode, MAPVK_VK_TO_VSC, GetKeyboardLayout(0));
			f32& value = *((f32*)data);
			if (key < 256) {
				value = _state[key] & 0x80 ? 1.f : 0.f;
			} else {
				value = 0.f;
			}
			*/
		}
		return 0;
	}

	void Gamepad::poll() {
		HRESULT hr = _dev->Poll();
		if (hr == DIERR_NOTACQUIRED || DIERR_INPUTLOST) {
			if (FAILED(_dev->Acquire())) return;
			if (FAILED(_dev->Poll())) return;
		}

		hr = _dev->GetDeviceState(sizeof(DIJOYSTATE2), &_pollState);
		if (SUCCEEDED(hr)) {
			/*
			ui16 changeBuffer[sizeof(StateBuffer)];
			ui16 len = 0;
			for (ui16 i = 0; i < sizeof(StateBuffer); ++i) {
				if (_state[i] != _pollState[i]) {
					_state[i] = _pollState[i];
					auto& kv = changeBuffer[len++];
					kv = (_state[i] << 8) | i;
				}
			}

			if (len > 0) {
				auto layout = GetKeyboardLayout(0);
				for (ui16 i = 0; i < len; ++i) {
					auto& data = changeBuffer[i];
					ui8 key = data & 0xFF;
					f32 value = (data >> 8 & 0x80) > 0 ? 1.f : 0.f;
					_eventDispatcher.dispatchEvent(this, value > 0 ? InputDeviceEvent::DOWN : InputDeviceEvent::UP, &InputKey({ MapVirtualKeyEx(key, MAPVK_VSC_TO_VK, layout), &value }));
				}
			}
			*/
		}
	}
}