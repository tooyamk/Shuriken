#include "Mouse.h"
#include "Input.h"

namespace srk::modules::inputs::direct_input {
	Mouse::Mouse(Input& input, LPDIRECTINPUTDEVICE8 dev, const DeviceInfo& info) : DeviceBase(input, dev, info) {
		_dev->SetDataFormat(&c_dfDIMouse2);
		_dev->SetCooperativeLevel(_input->getHWND(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
		memset(&_state, 0, sizeof(DIMOUSESTATE2));
		
		auto p = _getCursorPos();
		_pos = p.combined;
	}

	DeviceState::CountType Mouse::getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const {
		using namespace srk::enum_operators;

		switch (type) {
		case DeviceStateType::KEY:
		{
			if (values && count) {
				switch ((MouseKeyCode)code) {
				case MouseKeyCode::POSITION:
				{
					POINT p;
					GetCursorPos(&p);
					ScreenToClient(_input->getHWND(), &p);

					((DeviceStateValue*)values)[0] = DeviceStateValue(p.x);
					DeviceState::CountType c = 1;
					if (count > 1) ((DeviceStateValue*)values)[c++] = DeviceStateValue(p.y);

					return c;
				}
				case MouseKeyCode::WHEEL:
					return 0;
				default:
				{
					if (code >= MouseKeyCode::L_BUTTON && code < MouseKeyCode::L_BUTTON + sizeof(DIMOUSESTATE2::rgbButtons)) {
						std::shared_lock lock(_mutex);

						((DeviceStateValue*)values)[0] = _state.rgbButtons[code - 10] & 0x80 ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;

						return 1;
					}

					break;
				}
				}
			}

			return 0;
		}
		default:
			return 0;
		}
	}

	DeviceState::CountType Mouse::setState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count) {
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
			DeviceStateValue value[] = { (DeviceStateValue)ox, (DeviceStateValue)oy };
			DeviceState k = { (DeviceState::CodeType)MouseKeyCode::POSITION, 2, value };
			_eventDispatcher->dispatchEvent(this, DeviceEvent::MOVE, &k);
		}

		if (state.lZ != 0) {
			DeviceStateValue value = state.lZ > 0 ? Math::ONE<DeviceStateValue> : Math::NEGATIVE_ONE<DeviceStateValue>;
			DeviceState k = { (DeviceState::CodeType)MouseKeyCode::WHEEL, 1, &value };
			_eventDispatcher->dispatchEvent(this, DeviceEvent::MOVE, &k);
		}

		for (uint8_t i = 0; i < len; ++i) {
			DeviceState::CodeType key = changeBtns[i];
			DeviceStateValue value = (state.rgbButtons[key] & 0x80) > 0 ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;
			DeviceState k = { key + (DeviceState::CodeType)MouseKeyCode::L_BUTTON, 1, &value };
			_eventDispatcher->dispatchEvent(this, value > Math::ZERO<DeviceStateValue> ? DeviceEvent::DOWN : DeviceEvent::UP, &k);
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
		p.x = p2.x;
		p.y = p2.y;

		return p;
	}
}