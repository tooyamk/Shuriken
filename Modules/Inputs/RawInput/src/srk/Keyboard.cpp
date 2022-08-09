#include "Keyboard.h"

namespace srk::modules::inputs::raw_input {
	Keyboard::Keyboard(Input& input, windows::IWindow& win, const InternalDeviceInfo& info) : DeviceBase(input, win, info) {
		memset(_state, 0, sizeof(StateBuffer));
		memset(_listenState, 0, sizeof(StateBuffer));
	}

	DeviceState::CountType Keyboard::getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const {
		switch (type) {
		case DeviceStateType::KEY:
		{
			if (values && count && code < sizeof(_state)) {
				switch ((KeyboardVirtualKeyCode)code) {
				case KeyboardVirtualKeyCode::KEY_SHIFT:
					((DeviceStateValue*)values)[0] = (_state[(uint32_t)KeyboardVirtualKeyCode::KEY_LSHIFT]) || (_state[(uint32_t)KeyboardVirtualKeyCode::KEY_RSHIFT]) ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;
					return 1;
				case KeyboardVirtualKeyCode::KEY_CTRL:
					((DeviceStateValue*)values)[0] = (_state[(uint32_t)KeyboardVirtualKeyCode::KEY_LCTRL]) || (_state[(uint32_t)KeyboardVirtualKeyCode::KEY_RCTRL]) ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;
					return 1;
				case KeyboardVirtualKeyCode::KEY_ALT:
					((DeviceStateValue*)values)[0] = (_state[(uint32_t)KeyboardVirtualKeyCode::KEY_LALT]) || (_state[(uint32_t)KeyboardVirtualKeyCode::KEY_RALT]) ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;
					return 1;
				default:
					((DeviceStateValue*)values)[0] = _state[code] ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;
					return 1;
				}
			}
		}
		default:
			return 0;
		}
	}

	DeviceState::CountType Keyboard::setState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count) {
		return 0;
	}

	void Keyboard::poll(bool dispatchEvent) {
		StateBuffer state;
		{
			std::shared_lock lock(_listenMutex);

			memcpy(state, _listenState, sizeof(StateBuffer));
		}

		if (!dispatchEvent) {
			std::scoped_lock lock(_mutex);

			memcpy(_state, state, sizeof(StateBuffer));

			return;
		}

		StateBuffer changedBtns;
		uint16_t len = 0;

		{
			std::scoped_lock lock(_mutex);

			for (uint16_t i = 0; i < sizeof(StateBuffer); ++i) {
				if (_state[i] != state[i]) {
					_state[i] = state[i];
					changedBtns[len++] = uint8_t(i);
				}
			}
		}

		for (uint16_t i = 0; i < len; ++i) {
			auto key = changedBtns[i];
			DeviceStateValue value = state[key] ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;

			DeviceState k = { key, 1, &value };
			_eventDispatcher->dispatchEvent(this, value > Math::ZERO<DeviceStateValue> ? DeviceEvent::DOWN : DeviceEvent::UP, &k);
		}
	}

	void Keyboard::_rawInput(const RAWINPUT& rawInput) {
		auto& kb = rawInput.data.keyboard;
		switch (kb.Message) {
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			auto i = _getStateIndex(kb);

			std::scoped_lock lock(_listenMutex);

			_listenState[i] = 1;

			break;
		}
		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			auto i = _getStateIndex(kb);

			std::scoped_lock lock(_listenMutex);

			_listenState[i] = 0;

			break;
		}
		default:
			break;
		}
	}

	int32_t Keyboard::_getStateIndex(const RAWKEYBOARD& raw) {
		switch ((KeyboardVirtualKeyCode)raw.VKey) {
		case KeyboardVirtualKeyCode::KEY_SHIFT:
			return (int32_t)(raw.MakeCode == 0x2A ? KeyboardVirtualKeyCode::KEY_LSHIFT : KeyboardVirtualKeyCode::KEY_RSHIFT);
		case KeyboardVirtualKeyCode::KEY_CTRL:
			return (int32_t)(raw.Flags & RI_KEY_E0 ? KeyboardVirtualKeyCode::KEY_RCTRL : KeyboardVirtualKeyCode::KEY_LCTRL);
		case KeyboardVirtualKeyCode::KEY_ALT:
			return (int32_t)(raw.Flags & RI_KEY_E0 ? KeyboardVirtualKeyCode::KEY_RALT : KeyboardVirtualKeyCode::KEY_LALT);
		default:
			return raw.VKey;
		}
	}
}