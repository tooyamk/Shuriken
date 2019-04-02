#include "Gamepad.h"
#include "DirectInput.h"

namespace aurora::modules::win_direct_input {
	Gamepad::Gamepad(DirectInput* input, LPDIRECTINPUTDEVICE8 dev, const InputDeviceInfo& info) : DeviceBase(input, dev, info) {
		_dev->SetDataFormat(&c_dfDIJoystick2);
		_dev->SetCooperativeLevel(input->getHWND(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
		memset(&_state, 0, sizeof(DIJOYSTATE2));
		memset(&_state.rgdwPOV, 0xFF, sizeof(DIJOYSTATE2::rgdwPOV));
		_state.lX = 32767;
		_state.lY = 32767;
		_state.lZ = 32767;
		_state.lRz = 32767;
	}

	ui32 Gamepad::getKeyState(ui32 keyCode, f32* data, ui32 count) const {
		if (data && count) {
			switch (keyCode) {
			case 0:
			{
				data[0] = _translateStick(_state.lX);
				ui32 c = 1;
				if (count > 1) data[c++] = _translateStick(_state.lY);

				return c;
			}
			case 1:
			{
				data[0] = _translateStick(_state.lZ);
				ui32 c = 1;
				if (count > 1) data[c++] = _translateStick(_state.lRz);

				return c;
			}
			case 2:
			{
				data[0] = _translateTrigger(_state.lRx);

				return 1;
			}
			case 3:
			{
				data[0] = _translateTrigger(_state.lRy);

				return 1;
			}
			default:
			{
				if (keyCode >= 10 && keyCode < 14) {
					ui32 value = _state.rgdwPOV[keyCode - (ui32)10];
					data[0] = _translateAngle(_state.rgdwPOV[keyCode - (ui32)10]);

					return 1;
				} else if (keyCode >= 20 && keyCode < 148) {
					data[0] = _state.rgbButtons[keyCode - (ui32)20] & 0x80 ? 1.f : 0.f;

					return 1;
				}

				return 0;
			}
			}
		}
		return 0;
	}

	void Gamepad::poll(bool dispatchEvent) {
		HRESULT hr = _dev->Poll();
		if (hr == DIERR_NOTACQUIRED || DIERR_INPUTLOST) {
			if (FAILED(_dev->Acquire())) return;
			if (FAILED(_dev->Poll())) return;
		}

		if (!dispatchEvent) {
			_dev->GetDeviceState(sizeof(DIJOYSTATE2), &_state);
			return;
		}

		DIJOYSTATE2 state;
		hr = _dev->GetDeviceState(sizeof(DIJOYSTATE2), &state);
		if (SUCCEEDED(hr)) {
			bool l = false;
			if (_state.lX != state.lX || _state.lY != state.lY) {
				_state.lX = state.lX;
				_state.lY = state.lY;
				l = true;
			}
			bool r = false;
			if (_state.lZ != state.lZ || _state.lRz != state.lRz) {
				_state.lZ = state.lZ;
				_state.lRz = state.lRz;
				r = true;
			}

			bool lt = false, rt = false;
			if (_state.lRx != state.lRx) {
				_state.lRx = state.lRx;
				lt = true;
			}
			if (_state.lRy != state.lRy) {
				_state.lRy = state.lRy;
				rt = true;
			}

			ui8 changedBtns[sizeof(DIJOYSTATE2::rgbButtons)];
			ui8 changedBtnsLen = 0;
			for (ui8 i = 0; i < sizeof(DIJOYSTATE2::rgbButtons); ++i) {
				if (_state.rgbButtons[i] != state.rgbButtons[i]) {
					_state.rgbButtons[i] = state.rgbButtons[i];
					changedBtns[changedBtnsLen++] = i;
				}
			}

			ui8 changedPov[4];
			ui8 changedPovLen = 0;
			for (ui8 i = 0; i < sizeof(DIJOYSTATE2::rgdwPOV); ++i) {
				if (_state.rgdwPOV[i] != state.rgdwPOV[i]) {
					_state.rgdwPOV[i] = state.rgdwPOV[i];
					changedPov[changedPovLen++] = i;
				}
			}

			/*
			if (_state.lVX != state.lVX || _state.lVY != state.lVY || _state.lVZ != state.lVZ ||
				_state.lVRx != state.lVRx || _state.lVRy != state.lVRy || _state.lVRz != state.lVRz ||
				_state.lAX != state.lAX || _state.lAY != state.lAY || _state.lAZ != state.lAZ ||
				_state.lARx != state.lARx || _state.lARy != state.lARy || _state.lARz != state.lARz ||
				_state.lFX != state.lFX || _state.lFY != state.lFY || _state.lFZ != state.lFZ ||
				_state.lFRx != state.lARx || _state.lFRy != state.lFRy || _state.lFRz != state.lFRz) {
				_state.lVX = state.lVX;
				_state.lVY = state.lVY;
				_state.lVZ = state.lVZ;
				_state.lVRx = state.lVRx;
				_state.lVRy = state.lVRy;
				_state.lVRz = state.lVRz;
				_state.lAX = state.lAX;
				_state.lAY = state.lAY;
				_state.lAZ = state.lAZ;
				_state.lARx = state.lARx;
				_state.lARy = state.lARy;
				_state.lARz = state.lARz;
				_state.lFX = state.lFX;
				_state.lFY = state.lFY;
				_state.lFZ = state.lFZ;
				_state.lFRx = state.lFRx;
				_state.lFRy = state.lFRy;
				_state.lFRz = state.lFRz;
			}
			*/

			if (l) {
				f32 value[] = { _translateStick(state.lX) , _translateStick(state.lY) };
				_eventDispatcher.dispatchEvent(this, InputDeviceEvent::MOVE, &InputKey({ 0, 2, value }));
			}

			if (r) {
				f32 value[] = { _translateStick(state.lZ) , _translateStick(state.lRz) };
				_eventDispatcher.dispatchEvent(this, InputDeviceEvent::MOVE, &InputKey({ 1, 2, value }));
			}

			if (lt) {
				f32 value = _translateTrigger(state.lRx);
				_eventDispatcher.dispatchEvent(this, InputDeviceEvent::MOVE, &InputKey({ 2, 1, &value }));
			}

			if (rt) {
				f32 value = _translateTrigger(state.lRy);
				_eventDispatcher.dispatchEvent(this, InputDeviceEvent::MOVE, &InputKey({ 3, 1, &value }));
			}

			if (changedPovLen) {
				for (ui8 i = 0; i < changedPovLen; ++i) {
					ui8 key = changedPov[i];
					f32 value = _translateAngle(state.rgdwPOV[key]);
					_eventDispatcher.dispatchEvent(this, value >= 0.f ? InputDeviceEvent::DOWN : InputDeviceEvent::UP, &InputKey({ key + ui32(10), 1, &value }));
				}
			}

			if (changedBtnsLen) {
				for (ui8 i = 0; i < changedBtnsLen; ++i) {
					ui8 key = changedBtns[i];
					f32 value = (state.rgbButtons[key] & 0x80) > 0 ? 1.f : 0.f;
					_eventDispatcher.dispatchEvent(this, value > 0.f ? InputDeviceEvent::DOWN : InputDeviceEvent::UP, &InputKey({ key + ui32(20), 1, &value }));
				}
			}
		}
	}

	f32 Gamepad::_translateStick(LONG value) {
		auto v = f32(value - 32767);
		if (v < 0.f) {
			v /= 32767.f;
		} else if (v > 0.f) {
			v /= 32768.f;
		}
		return v;
	}

	f32 Gamepad::_translateTrigger(LONG value) {
		return f32(value) / 65535.f;
	}

	f32 Gamepad::_translateAngle(DWORD value) {
		return (value == 0xFFFFFFFFui32) ? -1.f : f32(value) * .01f;
	}
}