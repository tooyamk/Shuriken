#include "Keyboard.h"

namespace aurora::modules::inputs::raw_input {
	Keyboard::Keyboard(Input& input, IApplication& app, const InternalDeviceInfo& info) : DeviceBase(input, app, info) {
		memset(_state, 0, sizeof(StateBuffer));
		memset(_listenState, 0, sizeof(StateBuffer));
	}

	Key::CountType Keyboard::getKeyState(Key::CodeType keyCode, Key::ValueType* data, Key::CountType count) const {
		if (data && count && keyCode < sizeof(_state)) {
			switch ((KeyboardVirtualKeyCode)keyCode) {
			case KeyboardVirtualKeyCode::KEY_SHIFT:
				data[0] = (_state[(uint32_t)KeyboardVirtualKeyCode::KEY_LSHIFT]) || (_state[(uint32_t)KeyboardVirtualKeyCode::KEY_RSHIFT]) ? Math::ONE<Key::ValueType> : Math::ZERO<Key::ValueType>;
				return 1;
			case KeyboardVirtualKeyCode::KEY_CTRL:
				data[0] = (_state[(uint32_t)KeyboardVirtualKeyCode::KEY_LCTRL]) || (_state[(uint32_t)KeyboardVirtualKeyCode::KEY_RCTRL]) ? Math::ONE<Key::ValueType> : Math::ZERO<Key::ValueType>;
				return 1;
			case KeyboardVirtualKeyCode::KEY_ALT:
				data[0] = (_state[(uint32_t)KeyboardVirtualKeyCode::KEY_LALT]) || (_state[(uint32_t)KeyboardVirtualKeyCode::KEY_RALT]) ? Math::ONE<Key::ValueType> : Math::ZERO<Key::ValueType>;
				return 1;
			default:
				data[0] = _state[keyCode] ? Math::ONE<Key::ValueType> : Math::ZERO<Key::ValueType>;
				return 1;
			}
		}

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

		if (len > 0) {
			for (uint16_t i = 0; i < len; ++i) {
				auto key = changedBtns[i];
				Key::ValueType value = state[key] ? Math::ONE<Key::ValueType> : Math::ZERO<Key::ValueType>;

				Key k = { key, 1, &value };
				_eventDispatcher.dispatchEvent(this, value > Math::ZERO<Key::ValueType> ? DeviceEvent::DOWN : DeviceEvent::UP, &k);
			}
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