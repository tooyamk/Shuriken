#include "Mouse.h"
#include "Input.h"

namespace aurora::modules::inputs::direct_input {
	Mouse::Mouse(Input& input, LPDIRECTINPUTDEVICE8 dev, const DeviceInfo& info) : DeviceBase(input, dev, info) {
		_dev->SetDataFormat(&c_dfDIMouse2);
		_dev->SetCooperativeLevel(_input->getHWND(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
		memset(&_state, 0, sizeof(DIMOUSESTATE2));
		
		auto p = _getCursorPos();
		_pos = p.combined;
	}

	Key::CountType Mouse::getKeyState(Key::CodeType keyCode, Key::ValueType* data, Key::CountType count) const {
		using namespace aurora::enum_operators;

		if (data && count) {
			switch ((MouseKeyCode)keyCode) {
			case MouseKeyCode::POSITION:
			{
				POINT p;
				GetCursorPos(&p);
				ScreenToClient(_input->getHWND(), &p);

				data[0] = Key::ValueType(p.x);
				Key::CountType c = 1;
				if (count > 1) data[c++] = Key::ValueType(p.y);

				return c;
			}
			case MouseKeyCode::WHEEL:
				return 0;
			default:
			{
				if (keyCode >= MouseKeyCode::L_BUTTON && keyCode < MouseKeyCode::L_BUTTON + sizeof(DIMOUSESTATE2::rgbButtons)) {
					std::shared_lock lock(_mutex);

					data[0] = _state.rgbButtons[keyCode - 10] & 0x80 ? Math::ONE<Key::ValueType> : Math::ZERO<Key::ValueType>;

					return 1;
				}

				break;
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

		DIMOUSESTATE2 state;
		hr = _dev->GetDeviceState(sizeof(DIMOUSESTATE2), &state);
		if (!SUCCEEDED(hr)) return;

		auto p = _getCursorPos();

		if (!dispatchEvent) {
			{
				std::scoped_lock lock(_mutex);

				memcpy(&_state, &state, sizeof(DIMOUSESTATE2));
			}
			
			_pos = p.combined;

			return;
		}

		uint8_t changeBtns[sizeof(DIMOUSESTATE2::rgbButtons)];
		uint8_t len = 0;

		Point pos, lastPos;
		pos.combined = _pos.exchange(p.combined);
		int32_t ox = p.x - pos.x;
		int32_t oy = p.y - pos.y;

		{
			std::scoped_lock lock(_mutex);

			for (uint8_t i = 0; i < sizeof(DIMOUSESTATE2::rgbButtons); ++i) {
				if (_state.rgbButtons[i] != state.rgbButtons[i]) {
					_state.rgbButtons[i] = state.rgbButtons[i];
					changeBtns[len++] = i;
				}
			}
		}

		_amendmentRelativePos(ox, p.x, state.lX, SM_CXSCREEN);
		_amendmentRelativePos(oy, p.y, state.lY, SM_CYSCREEN);

		if (ox || oy) {
			//increment, right bottom positive orientation.
			Key::ValueType value[] = { (Key::ValueType)ox, (Key::ValueType)oy };
			Key k = { (Key::CodeType)MouseKeyCode::POSITION, 2, value };
			_eventDispatcher.dispatchEvent(this, DeviceEvent::MOVE, &k);
		}

		if (state.lZ != 0) {
			Key::ValueType value = state.lZ > 0 ? Math::ONE<Key::ValueType> : Math::NEGATIVE_ONE<Key::ValueType>;
			Key k = { (Key::CodeType)MouseKeyCode::WHEEL, 1, &value };
			_eventDispatcher.dispatchEvent(this, DeviceEvent::MOVE, &k);
		}

		for (uint8_t i = 0; i < len; ++i) {
			Key::CodeType key = changeBtns[i];
			Key::ValueType value = (state.rgbButtons[key] & 0x80) > 0 ? Math::ONE<Key::ValueType> : Math::ZERO<Key::ValueType>;
			Key k = { key + (Key::CodeType)MouseKeyCode::L_BUTTON, 1, &value };
			_eventDispatcher.dispatchEvent(this, value > Math::ZERO<Key::ValueType> ? DeviceEvent::DOWN : DeviceEvent::UP, &k);
		}
	}

	void Mouse::_amendmentRelativePos(int32_t& target, LONG absolutePos, LONG referenceRelativePos, int32_t nIndex) {
		if (target < 0) {
			if (absolutePos == 0 && referenceRelativePos < target) target = referenceRelativePos;
		} else if (target == 0) {
			if (absolutePos == 0) {
				if (referenceRelativePos < 0) target = referenceRelativePos;
			} else if (absolutePos == GetSystemMetrics(nIndex) - 1) {
				if (referenceRelativePos > 0) target = referenceRelativePos;
			}
		} else if (target > 0) {
			if (absolutePos == GetSystemMetrics(nIndex) - 1 && referenceRelativePos > target) target = referenceRelativePos;
		}
	}

	Mouse::Point Mouse::_getCursorPos() {
		Point p;

		POINT p2;
		GetCursorPos(&p2);
		p.x = p.x;
		p.y = p.y;

		return p;
	}
}