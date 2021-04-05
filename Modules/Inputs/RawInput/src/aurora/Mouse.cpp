#include "Mouse.h"
#include "Input.h"

namespace aurora::modules::inputs::raw_input {
	Mouse::Mouse(Input& input, IApplication& app, const InternalDeviceInfo& info) : DeviceBase(input, app, info),
		_listenLastPos(0),
		_lastWheel(0) {
		memset(_state, 0, sizeof(StateBuffer));
		memset(_listenState, 0, sizeof(StateBuffer));
		GetCursorPos(&_pos);
	}

	Key::CountType Mouse::getKeyState(Key::CodeType keyCode, Key::ValueType* data, Key::CountType count) const {
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
				if (keyCode >= (uint32_t)MouseKeyCode::L_BUTTON && keyCode < (uint32_t)MouseKeyCode::L_BUTTON + sizeof(StateBuffer)) {
					std::shared_lock lock(_mutex);

					data[0] = _state[keyCode - (uint32_t)MouseKeyCode::L_BUTTON] ? Math::ONE<Key::ValueType> : Math::ZERO<Key::ValueType>;

					return 1;
				}

				break;
			}
			}
		}

		return 0;
	}

	void Mouse::poll(bool dispatchEvent) {
		POINT p;
		GetCursorPos(&p);

		if (!dispatchEvent) {
			std::scoped_lock lock(_mutex);
			std::shared_lock lock2(_listenMutex);

			memcpy(_state, _listenState, sizeof(StateBuffer));
			_pos = p;

			return;
		}

		StateBuffer state;
		{
			std::shared_lock lock(_listenMutex);

			memcpy(state, _listenState, sizeof(StateBuffer));
		}

		StateBuffer changeBtns;
		uint8_t len = 0;

		int32_t ox, oy;
		int32_t wheel = _lastWheel.exchange(0);
		uint64_t lastPos = _listenLastPos.exchange(0);
		auto lastPosXY = (int32_t*)&lastPos;

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

		_amendmentRelativePos(ox, p.x, lastPosXY[0], SM_CXSCREEN);
		_amendmentRelativePos(oy, p.y, lastPosXY[1], SM_CYSCREEN);

		if (ox || oy) {
			//increment, right bottom positive orientation.
			Key::ValueType value[] = { (Key::ValueType)ox, (Key::ValueType)oy };
			Key k = { (Key::CodeType)MouseKeyCode::POSITION, 2, value };
			_eventDispatcher.dispatchEvent(this, DeviceEvent::MOVE, &k);
		}

		if (wheel != 0) {
			Key::ValueType value = (Key::ValueType)wheel * Math::RECIPROCAL<Key::ValueType(WHEEL_DELTA)>;
			Key k = { (Key::CodeType)MouseKeyCode::WHEEL, 1, &value };
			_eventDispatcher.dispatchEvent(this, DeviceEvent::MOVE, &k);
		}

		for (uint8_t i = 0; i < len; ++i) {
			Key::CodeType key = changeBtns[i];
			Key::ValueType value = state[key] ? Math::ONE<Key::ValueType> : Math::ZERO<Key::ValueType>;
			Key k = { key + (Key::CodeType)MouseKeyCode::L_BUTTON, 1, &value };
			_eventDispatcher.dispatchEvent(this, value > 0 ? DeviceEvent::DOWN : DeviceEvent::UP, &k);
		}
	}

	void Mouse::_rawInput(const RAWINPUT& rawInput) {
		using namespace enum_operators;

		auto& m = rawInput.data.mouse;
		
		constexpr size_t n = sizeof(m.usButtonFlags) * 8;
		for (size_t i = 0; i < n; ++i) {
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
				_lastWheel = (SHORT)m.usButtonData;
				break;
			default:
				break;
			}

			using I = typename std::underlying_type_t<MouseKeyCode>;
			if (I i = (I)key - (I)MouseKeyCode::L_BUTTON; i < sizeof(StateBuffer)) {
				std::scoped_lock lock(_listenMutex);

				_listenState[i] = val;
			}
		}

		uint64_t pos;
		auto xy = (int32_t*)&pos;
		if (m.usFlags == MOUSE_MOVE_RELATIVE) {
			xy[0] = m.lLastX;
			xy[1] = m.lLastY;
		} else {
			xy[0] = 0;
			xy[1] = 0;
		}
		_listenLastPos = pos;
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