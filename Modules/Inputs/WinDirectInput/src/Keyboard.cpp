#include "Keyboard.h"
#include "DirectInput.h"

namespace aurora::modules::win_direct_input {
	Keyboard::Keyboard(DirectInput* input, LPDIRECTINPUTDEVICE8 dev, const InputDeviceInfo& info) : DeviceBase(input, dev, info) {
		_dev->SetDataFormat(&c_dfDIKeyboard);
		_dev->SetCooperativeLevel(input->getHWND(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
		memset(_state, 0, sizeof(StateBuffer));
	}

	ui32 Keyboard::getKeyState(ui32 keyCode, f32* data, ui32 count) const {
		if (data && count) {
			ui32 key = MapVirtualKeyEx(keyCode, MAPVK_VK_TO_VSC, GetKeyboardLayout(0));
			if (key < 256) {
				data[0] = _state[key] & 0x80 ? 1.f : 0.f;

				return 1;
			}
		}
		return 0;
	}

	void Keyboard::poll() {
		HRESULT hr = _dev->Poll();
		if (hr == DIERR_NOTACQUIRED || DIERR_INPUTLOST) {
			if (FAILED(_dev->Acquire())) return;
			if (FAILED(_dev->Poll())) return;
		}

		StateBuffer state;
		hr = _dev->GetDeviceState(sizeof(StateBuffer), state);
		if (SUCCEEDED(hr)) {
			StateBuffer changedBtns;
			ui16 len = 0;
			for (ui16 i = 0; i < sizeof(StateBuffer); ++i) {
				if (_state[i] != state[i]) {
					_state[i] = state[i];
					changedBtns[len++] = i;
				}
			}

			if (len > 0) {
				auto layout = GetKeyboardLayout(0);
				for (ui16 i = 0; i < len; ++i) {
					ui8 key = changedBtns[i];
					f32 value = (state[key] & 0x80) > 0 ? 1.f : 0.f;
					_eventDispatcher.dispatchEvent(this, value > 0.f ? InputDeviceEvent::DOWN : InputDeviceEvent::UP, &InputKey({ MapVirtualKeyEx(key, MAPVK_VSC_TO_VK, layout), 1, &value }));
				}
			}
		}
	}
}