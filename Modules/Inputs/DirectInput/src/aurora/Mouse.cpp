#include "Mouse.h"
#include "Input.h"

namespace aurora::modules::inputs::direct_input {
	Mouse::Mouse(Input& input, LPDIRECTINPUTDEVICE8 dev, const DeviceInfo& info) : DeviceBase(input, dev, info) {
		_dev->SetDataFormat(&c_dfDIMouse2);
		_dev->SetCooperativeLevel(_input->getHWND(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
		memset(&_state, 0, sizeof(DIMOUSESTATE2));

		GetCursorPos(&_pos);
	}

	uint32_t Mouse::getKeyState(uint32_t keyCode, float32_t* data, uint32_t count) const {
		if (data && count) {
			switch (keyCode) {
			case (uint32_t)MouseKeyCode::POSITION:
			{
				auto p = _getClientPos();
				data[0] = float32_t(p.x);
				uint32_t c = 1;
				if (count > 1) data[c++] = float32_t(p.y);

				return c;
			}
			case (uint32_t)MouseKeyCode::WHEEL:
				return 0;
			default:
			{
				if (keyCode >= (uint32_t)MouseKeyCode::L_BUTTON && keyCode < (uint32_t)MouseKeyCode::L_BUTTON + sizeof(DIMOUSESTATE2::rgbButtons)) {
					std::shared_lock lock(_mutex);

					data[0] = _state.rgbButtons[keyCode - (uint32_t)10] & 0x80 ? 1.f : 0.f;

					return 1;
				}

				return 0;
			}
			}
		}
		return 0;
	}

	void Mouse::poll(bool dispatchEvent) {
		HRESULT hr = _dev->Poll();
		if (hr == DIERR_NOTACQUIRED || hr == DIERR_INPUTLOST) {
			if (FAILED(_dev->Acquire())) return;
			if (FAILED(_dev->Poll())) return;
		}

		if (!dispatchEvent) {
			std::scoped_lock lock(_mutex);

			_dev->GetDeviceState(sizeof(DIJOYSTATE2), &_state);
			return;
		}

		DIMOUSESTATE2 state;
		hr = _dev->GetDeviceState(sizeof(DIMOUSESTATE2), &state);
		if (SUCCEEDED(hr)) {
			uint8_t changeBtns[sizeof(DIMOUSESTATE2::rgbButtons)];
			uint8_t len = 0;

			POINT p;
			GetCursorPos(&p);
			int32_t ox, oy;

			{
				std::scoped_lock lock(_mutex);

				for (uint8_t i = 0; i < sizeof(DIMOUSESTATE2::rgbButtons); ++i) {
					if (_state.rgbButtons[i] != state.rgbButtons[i]) {
						_state.rgbButtons[i] = state.rgbButtons[i];
						changeBtns[len++] = i;
					}
				}

				ox = p.x - _pos.x;
				oy = p.y - _pos.y;

				_pos = p;
			}

			if (ox < 0) {
				if (p.x == 0 && state.lX < ox) ox = state.lX;
			} else if (ox == 0) {
				if (p.x == 0) {
					if (state.lX < 0) ox = state.lX;
				} else if (p.x == GetSystemMetrics(SM_CXSCREEN) - 1) {
					if (state.lX > 0) ox = state.lX;
				}
			} else if (ox > 0) {
				if (p.x == GetSystemMetrics(SM_CXSCREEN) - 1 && state.lX > ox) ox = state.lX;
			}

			if (oy < 0) {
				if (p.y == 0 && state.lY < oy) oy = state.lY;
			} else if (oy == 0) {
				if (p.y == 0) {
					if (state.lY < 0) oy = state.lY;
				} else if (p.x == GetSystemMetrics(SM_CYSCREEN) - 1) {
					if (state.lY > 0) oy = state.lY;
				}
			} else if (oy > 0) {
				if (p.y == GetSystemMetrics(SM_CYSCREEN) - 1 && state.lY > oy) oy = state.lY;
			}

			if (ox || oy) {
				//increment, right bottom positive orientation.
				float32_t value[] = { (float32_t)ox, (float32_t)oy };
				Key k = { (uint32_t)MouseKeyCode::POSITION, 2, value };
				_eventDispatcher.dispatchEvent(this, DeviceEvent::MOVE, &k);
			}

			if (state.lZ != 0) {
				float32_t value = state.lZ > 0 ? 1.f : -1.f;
				Key k = { (uint32_t)MouseKeyCode::WHEEL, 1, &value };
				_eventDispatcher.dispatchEvent(this, DeviceEvent::MOVE, &k);
			}

			for (uint8_t i = 0; i < len; ++i) {
				uint8_t key = changeBtns[i];
				float32_t value = (state.rgbButtons[key] & 0x80) > 0 ? 1.f : 0.f;
				Key k = { key + (uint32_t)MouseKeyCode::L_BUTTON, 1, &value };
				_eventDispatcher.dispatchEvent(this, value > 0 ? DeviceEvent::DOWN : DeviceEvent::UP, &k);
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