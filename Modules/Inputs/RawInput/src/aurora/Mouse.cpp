#include "Mouse.h"
#include "aurora/Debug.h"

namespace aurora::modules::inputs::raw_input {
	Mouse::Mouse(Input& input, IApplication& app, const InternalDeviceInfo& info) : DeviceBase(input, app, info) {
		memset(_state, 0, sizeof(StateBuffer));
		memset(_listenState, 0, sizeof(StateBuffer));
		GetCursorPos(&_pos);
		memset(&_listenLastPos, 0, sizeof(POINT));
	}

	uint32_t Mouse::getKeyState(uint32_t keyCode, float32_t* data, uint32_t count) const {
		return 0;
	}

	void Mouse::poll(bool dispatchEvent) {
		if (!dispatchEvent) {
			std::scoped_lock lock(_mutex);
			std::shared_lock lock2(_listenMutex);

			memcpy(_state, _listenState, sizeof(StateBuffer));
			return;
		}

		StateBuffer state;
		POINT lastPos;
		{
			std::shared_lock lock(_listenMutex);

			memcpy(state, _listenState, sizeof(StateBuffer));
			lastPos = _listenLastPos;
		}

		StateBuffer changeBtns;
		uint8_t len = 0;

		POINT p;
		GetCursorPos(&p);
		int32_t ox, oy;

		{
			std::scoped_lock lock(_mutex);

			for (uint8_t i = 0; i < sizeof(StateBuffer); ++i) {
				if (_state[i] != state[i]) {
					_state[i] = state[i];
					changeBtns[len++] = i;
				}
			}

			ox = p.x - _pos.x;
			oy = p.y - _pos.y;

			_pos = p;
		}

		_amendmentRelativePos(ox, p.x, lastPos.x, SM_CXSCREEN);
		_amendmentRelativePos(oy, p.y, lastPos.y, SM_CYSCREEN);

		if (ox || oy) {
			//increment, right bottom positive orientation.
			float32_t value[] = { (float32_t)ox, (float32_t)oy };
			Key k = { (uint32_t)MouseKeyCode::POSITION, 2, value };
			_eventDispatcher.dispatchEvent(this, DeviceEvent::MOVE, &k);
		}

		for (uint8_t i = 0; i < len; ++i) {
			uint8_t key = changeBtns[i];
			float32_t value = state[key] ? 1.f : 0.f;
			Key k = { key + (uint32_t)MouseKeyCode::L_BUTTON, 1, &value };
			_eventDispatcher.dispatchEvent(this, value > 0 ? DeviceEvent::DOWN : DeviceEvent::UP, &k);
		}
	}

	void Mouse::_rawInput(const RAWINPUT& rawInput) {
		using namespace enum_operators;

		auto& m = rawInput.data.mouse;
		
		for (size_t i = 0; i < sizeof(m.usButtonFlags); ++i) {
			decltype(m.usButtonFlags) flag = 1 << i;
			if ((m.usButtonFlags & flag) != flag) continue;

			auto key = (MouseKeyCode)-1;
			uint8_t val;
			switch (flag) {
			case RI_MOUSE_LEFT_BUTTON_DOWN:
			{
				key = MouseKeyCode::L_BUTTON;
				val = 1;

				break;
			}
			case RI_MOUSE_LEFT_BUTTON_UP:
			{
				key = MouseKeyCode::L_BUTTON;
				val = 0;

				break;
			}
			case RI_MOUSE_RIGHT_BUTTON_DOWN:
			{
				key = MouseKeyCode::R_BUTTON;
				val = 1;

				break;
			}
			case RI_MOUSE_RIGHT_BUTTON_UP:
			{
				key = MouseKeyCode::R_BUTTON;
				val = 0;

				break;
			}
			case RI_MOUSE_MIDDLE_BUTTON_DOWN:
			{
				key = MouseKeyCode::M_BUTTON;
				val = 1;

				break;
			}
			case RI_MOUSE_MIDDLE_BUTTON_UP:
			{
				key = MouseKeyCode::M_BUTTON;
				val = 0;

				break;
			}
			case RI_MOUSE_BUTTON_4_DOWN:
			{
				key = MouseKeyCode::FN_BUTTON_0;
				val = 1;

				break;
			}
			case RI_MOUSE_BUTTON_4_UP:
			{
				key = MouseKeyCode::FN_BUTTON_0;
				val = 0;

				break;
			}
			case RI_MOUSE_BUTTON_5_DOWN:
			{
				key = MouseKeyCode::FN_BUTTON_0 + 1;
				val = 1;

				break;
			}
			case RI_MOUSE_BUTTON_5_UP:
			{
				key = MouseKeyCode::FN_BUTTON_0 + 1;
				val = 0;

				break;
			}
			case RI_MOUSE_WHEEL:
			{
				auto numTicks = (float32_t)(SHORT)m.usButtonData / WHEEL_DELTA;

				break;
			}
			default:
				break;
			}

			using I = typename std::underlying_type_t<MouseKeyCode>;
			if (I i = (I)key - (I)MouseKeyCode::L_BUTTON; i < sizeof(StateBuffer)) {
				std::scoped_lock lock(_listenMutex);

				_listenState[i] = val;
			}
		}

		POINT p;
		if (m.usFlags == MOUSE_MOVE_RELATIVE) {
			p.x = m.lLastX;
			p.y = m.lLastY;
		} else {
			p.x = 0;
			p.y = 0;
		}

		std::scoped_lock lock(_listenMutex);

		_listenLastPos = p;
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
}