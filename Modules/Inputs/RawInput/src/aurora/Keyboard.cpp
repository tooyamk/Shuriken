#include "Keyboard.h"

namespace aurora::modules::inputs::raw_input {
	Keyboard::Keyboard(Input& input, IApplication& app, const InternalDeviceInfo& info) : DeviceBase(input, app, info) {
		memset(_state, 0, sizeof(StateBuffer));
		memset(_listenState, 0, sizeof(StateBuffer));
	}

	uint32_t Keyboard::getKeyState(uint32_t keyCode, float32_t* data, uint32_t count) const {
		if (data && count && keyCode < sizeof(_state)) {
			switch (keyCode) {
			case (uint32_t)KeyboardVirtualKeyCode::KEY_SHIFT:
				data[0] = (_state[(uint32_t)KeyboardVirtualKeyCode::KEY_LSHIFT]) || (_state[(uint32_t)KeyboardVirtualKeyCode::KEY_RSHIFT]) ? 1.f : 0.f;
				return 1;
			case (uint32_t)KeyboardVirtualKeyCode::KEY_CTRL:
				data[0] = (_state[(uint32_t)KeyboardVirtualKeyCode::KEY_LCTRL]) || (_state[(uint32_t)KeyboardVirtualKeyCode::KEY_RCTRL]) ? 1.f : 0.f;
				return 1;
			case VK_MENU:
				//data[0] = (_state[VK_SK[VK_LMENU]] & 0x80) || (_state[VK_SK[VK_RMENU]] & 0x80) ? 1.f : 0.f;
				return 1;
			default:
				data[0] = _state[keyCode] ? 1.f : 0.f;
				return 1;
			}
		}

		return 0;
	}

	void Keyboard::poll(bool dispatchEvent) {
		if (!dispatchEvent) {
			std::scoped_lock lock(_mutex);
			std::shared_lock lock2(_listenMutex);

			memcpy(_state, _listenState, sizeof(StateBuffer));
			return;
		}

		StateBuffer state;
		{
			std::shared_lock lock2(_listenMutex);

			memcpy(state, _listenState, sizeof(StateBuffer));
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
				uint8_t key = changedBtns[i];
				float32_t value = state[key] ? 1.f : 0.f;

				Key k = { key, 1, &value };
				_eventDispatcher.dispatchEvent(this, value > 0.f ? DeviceEvent::DOWN : DeviceEvent::UP, &k);
			}
		}
	}

	void Keyboard::_rawInput(const RAWINPUT& rawInput) {
		auto& kb = rawInput.data.keyboard;
		switch (kb.Message) {
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			switch (kb.VKey) {
			case (USHORT)KeyboardVirtualKeyCode::KEY_SHIFT:
			}
			std::scoped_lock lock(_listenMutex);

			_listenState[kb.VKey] = 1;

			break;
		}
		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			std::scoped_lock lock(_listenMutex);

			_listenState[kb.VKey] = 0;

			break;
		}
		default:
			break;
		}
	}
}