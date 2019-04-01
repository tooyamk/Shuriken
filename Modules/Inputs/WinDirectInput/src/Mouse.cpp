#include "Mouse.h"
#include "DirectInput.h"
#include "math/Vector2.h"
#include <algorithm>

namespace aurora::modules::win_direct_input {
	Mouse::Mouse(DirectInput* input, LPDIRECTINPUTDEVICE8 dev, const InputDeviceInfo& info) : DeviceBase(input, dev, info) {
		_dev->SetDataFormat(&c_dfDIMouse2);
		_dev->SetCooperativeLevel(input->getHWND(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
		memset(&_state, 0, sizeof(DIMOUSESTATE2));

		GetCursorPos(&_pos);
	}

	ui32 Mouse::getKeyState(ui32 keyCode, f32* data, ui32 count) const {
		if (data && count) {
			switch (keyCode) {
			case 0:
			{
				auto p = _getClientPos();
				ui32 c = 0;
				data[c++] = f32(p.x);
				if (count > 1) data[c++] = f32(p.y);

				return c;
			}
			case 1:
				return 0;
			default:
			{
				if (keyCode > 9 && keyCode < 18) {
					data[0] = _state.rgbButtons[keyCode - (ui32)10] & 0x80 ? 1.f : 0.f;
					return 1;
				}

				return 0;
			}
			}
		}
		return 0;
	}

	void Mouse::poll() {
		HRESULT hr = _dev->Poll();
		if (hr == DIERR_NOTACQUIRED || DIERR_INPUTLOST) {
			if (FAILED(_dev->Acquire())) return;
			if (FAILED(_dev->Poll())) return;
		}

		hr = _dev->GetDeviceState(sizeof(DIMOUSESTATE2), &_pollState);
		if (SUCCEEDED(hr)) {
			f32 z = _pollState.lZ > 0 ? 1.f : (_pollState.lZ < 0 ? -1.f : 0.f);

			ui16 changeBtns[sizeof(DIMOUSESTATE2::rgbButtons)];
			ui8 len = 0;
			for (ui8 i = 0; i < sizeof(DIMOUSESTATE2::rgbButtons); ++i) {
				if (_state.rgbButtons[i] != _pollState.rgbButtons[i]) {
					_state.rgbButtons[i] = _pollState.rgbButtons[i];
					auto& kv = changeBtns[len++];
					kv = (_state.rgbButtons[i] << 8) | i;
				}
			}

			POINT p;
			GetCursorPos(&p);
			i32 ox = p.x - _pos.x, oy = p.y - _pos.y;
			if (ox < 0) {
				if (p.x == 0 && _pollState.lX < ox) ox = _pollState.lX;
			} else if (ox == 0) {
				if (p.x == 0) {
					if (_pollState.lX < 0) ox = _pollState.lX;
				} else if (p.x == GetSystemMetrics(SM_CXSCREEN) - 1) {
					if (_pollState.lX > 0) ox = _pollState.lX;
				}
			} else if (ox > 0) {
				if (p.x == GetSystemMetrics(SM_CXSCREEN) - 1 && _pollState.lX > ox) ox = _pollState.lX;
			}

			if (oy < 0) {
				if (p.y == 0 && _pollState.lY < oy) oy = _pollState.lY;
			} else if (oy == 0) {
				if (p.y == 0) {
					if (_pollState.lY < 0) oy = _pollState.lY;
				} else if (p.x == GetSystemMetrics(SM_CYSCREEN) - 1) {
					if (_pollState.lY > 0) oy = _pollState.lY;
				}
			} else if (oy > 0) {
				if (p.y == GetSystemMetrics(SM_CYSCREEN) - 1 && _pollState.lY > oy) oy = _pollState.lY;
			}
			_pos = p;

			if (ox || oy) {
				//increment, right bottom positive orientation.
				f32 value[2] = { (f32)ox, (f32)oy };
				_eventDispatcher.dispatchEvent(this, InputDeviceEvent::MOVE, &InputKey({ 0, 2, value }));
			}

			if (z != 0.f) _eventDispatcher.dispatchEvent(this, InputDeviceEvent::MOVE, &InputKey({ 1, 1, &z }));

			for (ui8 i = 0; i < len; ++i) {
				auto& kv = changeBtns[i];
				ui8 key = kv & 0xFF;
				f32 value = (kv >> 8 & 0x80) > 0 ? 1.f : 0.f;
				_eventDispatcher.dispatchEvent(this, value > 0 ? InputDeviceEvent::DOWN : InputDeviceEvent::UP, &InputKey({ key + (ui32)10, 1, &value }));
			}
		}
	}

	POINT Mouse::_getClientPos() const {
		POINT p;
		GetCursorPos(&p);
		ScreenToClient(_input->getHWND(), &p);
		return p;
	}
}