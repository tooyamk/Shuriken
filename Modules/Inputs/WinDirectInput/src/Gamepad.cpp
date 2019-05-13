#include "Gamepad.h"
#include "DirectInput.h"
#include "math/Math.h"

namespace aurora::modules::win_direct_input {
	Gamepad::Gamepad(DirectInput* input, LPDIRECTINPUTDEVICE8 dev, const InputDeviceInfo& info) : DeviceBase(input, dev, info),
		_specifiedKeyToEnumMapping(nullptr) {
		_dev->SetDataFormat(&c_dfDIJoystick2);
		_dev->SetCooperativeLevel(input->getHWND(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
		memset(&_state, 0, sizeof(_state));
		memset(&_state.rgdwPOV, 0xFF, sizeof(_state.rgdwPOV));
		_state.lX = 32767;
		_state.lY = 32767;
		_state.lZ = 32767;
		_state.lRz = 32767;

		auto data = (const ui16*)info.guid.getData();
		auto vender = data[0];
		auto product = data[1];
		switch (data[0]) {
		case 0x054C:
		{
			if (product == 0x05C4 || product == 0x09CC) {
				_specifiedKeyToEnumMapping = &Gamepad::DS4;
			}

			break;
		}
		default:
			break;
		}

		if (_specifiedKeyToEnumMapping) {
			for (auto& itr : *_specifiedKeyToEnumMapping) _specifiedEnumToKeyMapping.try_emplace(itr.second, itr.first);
		}

		setDeadZone((ui8)GamepadKeyCode::LEFT_STICK, .05f);
		setDeadZone((ui8)GamepadKeyCode::RIGHT_STICK, .05f);
		setDeadZone((ui8)GamepadKeyCode::LEFT_TRIGGER, .05f);
		setDeadZone((ui8)GamepadKeyCode::RIGHT_TRIGGER, .05f);
	}

	ui32 Gamepad::getKeyState(ui32 keyCode, f32* data, ui32 count) const {
		if (data && count) {
			switch ((GamepadKeyCode)keyCode) {
			case GamepadKeyCode::LEFT_STICK:
				return _getStick(_state.lX, _state.lY, (GamepadKeyCode)keyCode, data, count);
			case GamepadKeyCode::RIGHT_STICK:
				return _getStick(_state.lZ, _state.lRz, (GamepadKeyCode)keyCode, data, count);
			case GamepadKeyCode::LEFT_TRIGGER:
				return _getTrigger(_state.lRx, (GamepadKeyCode)keyCode, data[0]);
			case GamepadKeyCode::RIGHT_TRIGGER:
				return _getTrigger(_state.lRy, (GamepadKeyCode)keyCode, data[0]);
			case GamepadKeyCode::DPAD:
				data[0] = _translateAngle(_state.rgdwPOV[0]);
				return 1;
			case GamepadKeyCode::X:
				data[0] = _translateButton(_state.rgbButtons[0]);
				return 1;
			case GamepadKeyCode::A:
				data[0] = _translateButton(_state.rgbButtons[1]);
				return 1;
			case GamepadKeyCode::B:
				data[0] = _translateButton(_state.rgbButtons[2]);
				return 1;
			case GamepadKeyCode::Y:
				data[0] = _translateButton(_state.rgbButtons[3]);
				return 1;
			case GamepadKeyCode::LEFT_SHOULDER:
				data[0] = _translateButton(_state.rgbButtons[4]);
				return 1;
			case GamepadKeyCode::RIGHT_SHOULDER:
				data[0] = _translateButton(_state.rgbButtons[5]);
				return 1;
			case GamepadKeyCode::SELECT:
				data[0] = _translateButton(_state.rgbButtons[8]);
				return 1;
			case GamepadKeyCode::START:
				data[0] = _translateButton(_state.rgbButtons[9]);
				return 1;
			default:
			{
				if (_specifiedKeyToEnumMapping) {
					auto itr = _specifiedEnumToKeyMapping.find((GamepadKeyCode)keyCode);
					if (itr != _specifiedEnumToKeyMapping.end()) {
						data[0] = _translateButton(_state.rgbButtons[itr->second]);

						return 1;
					}
				}

				if (keyCode >= (ui8)GamepadKeyCode::UNDEFINED) {
					data[0] = _translateButton(_state.rgbButtons[keyCode + 11 - (ui8)GamepadKeyCode::UNDEFINED]);

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
			_dev->GetDeviceState(sizeof(_state), &_state);
			return;
		}

		DIJOYSTATE2 state;
		hr = _dev->GetDeviceState(sizeof(state), &state);
		if (SUCCEEDED(hr)) {
			auto ls = _state.lX != state.lX || _state.lY != state.lY;
			auto rs = _state.lZ != state.lZ || _state.lRz != state.lRz;
			auto lt = _state.lRx != state.lRx;
			auto rt = _state.lRy != state.lRy;

			ui8 changedBtns[sizeof(state.rgbButtons)];
			ui8 changedBtnsLen = 0;
			for (ui8 i = 0; i < sizeof(state.rgbButtons); ++i) {
				if (_state.rgbButtons[i] != state.rgbButtons[i]) {
					_state.rgbButtons[i] = state.rgbButtons[i];
					changedBtns[changedBtnsLen++] = i;
				}
			}

			ui8 changedPov[4];
			ui8 changedPovLen = 0;
			for (ui8 i = 0; i < sizeof(state.rgdwPOV); ++i) {
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

			if (ls) _updateStick(_state.lX, _state.lY, state.lX, state.lY, GamepadKeyCode::LEFT_STICK);
			if (rs) _updateStick(_state.lZ, _state.lRz, state.lZ, state.lRz, GamepadKeyCode::RIGHT_STICK);
			if (lt) _updateTrigger(_state.lRx, state.lRx, GamepadKeyCode::LEFT_TRIGGER);
			if (rt) _updateTrigger(_state.lRy, state.lRy, GamepadKeyCode::RIGHT_TRIGGER);

			if (changedPovLen) {
				for (ui8 i = 0; i < changedPovLen; ++i) {
					ui8 key = changedPov[i];
					if (key == 0) {
						f32 value = _translateAngle(state.rgdwPOV[key]);
						_eventDispatcher.dispatchEvent(this, value >= 0.f ? InputDeviceEvent::DOWN : InputDeviceEvent::UP, &InputKey({ (ui8)GamepadKeyCode::DPAD, 1, &value }));
					}
				}
			}

			if (changedBtnsLen) {
				for (ui8 i = 0; i < changedBtnsLen; ++i) {
					ui8 key = changedBtns[i];
					f32 value = _translateButton(state.rgbButtons[key]);
					switch (key) {
					case 0:
						key = (ui8)GamepadKeyCode::X;
						break;
					case 1:
						key = (ui8)GamepadKeyCode::A;
						break;
					case 2:
						key = (ui8)GamepadKeyCode::B;
						break;
					case 3:
						key = (ui8)GamepadKeyCode::Y;
						break;
					case 4:
						key = (ui8)GamepadKeyCode::LEFT_SHOULDER;
						break;
					case 5:
						key = (ui8)GamepadKeyCode::RIGHT_SHOULDER;
						break;
					case 6:
						key = (ui8)GamepadKeyCode::LEFT_TRIGGER;
						break;
					case 7:
						key = (ui8)GamepadKeyCode::RIGHT_TRIGGER;
						break;
					case 8:
						key = (ui8)GamepadKeyCode::SELECT;
						break;
					case 9:
						key = (ui8)GamepadKeyCode::START;
						break;
					case 10:
						key = (ui8)GamepadKeyCode::LEFT_STICK;
						break;
					case 11:
						key = (ui8)GamepadKeyCode::RIGHT_STICK;
						break;
					default:
					{
						if (_specifiedKeyToEnumMapping) {
							auto itr = _specifiedKeyToEnumMapping->find(key);
							if (itr != _specifiedKeyToEnumMapping->end()) {
								key = (ui8)itr->second;

								break;
							}
						}

						key = key - 11 + (ui8)GamepadKeyCode::UNDEFINED;
						break;
					}
					}
					_eventDispatcher.dispatchEvent(this, value > 0.f ? InputDeviceEvent::DOWN : InputDeviceEvent::UP, &InputKey({ key, 1, &value }));
				}
			}
		}
	}

	void Gamepad::setDeadZone(ui32 keyCode, f32 deadZone) {
		if (deadZone < 0.f) deadZone = -deadZone;

		if (auto itr = _deadZone.find(keyCode); itr == _deadZone.end()) {
			_deadZone.emplace(keyCode, deadZone);
		} else {
			itr->second = deadZone;
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

	f32 Gamepad::_translateButton(DWORD value) {
		return value & 0x80 ? 1.f : 0.f;
	}

	ui32 Gamepad::_getStick(LONG x, LONG y, GamepadKeyCode key, f32* data, ui32 count) const {
		auto dz = _getDeadZone(key);

		data[0] = _translateStick(x);
		_translateDeadZone_1_1(data[0], dz, Math::isEqual(data[0], 0.f, dz));
		ui32 c = 1;
		if (count > 1) {
			data[c++] = _translateStick(y);
			_translateDeadZone_1_1(data[1], dz, Math::isEqual(data[1], 0.f, dz));
		}

		return c;
	}

	ui32 Gamepad::_getTrigger(LONG t, GamepadKeyCode key, f32& data) const {
		auto dz = _getDeadZone(key);

		data = _translateTrigger(t);
		_translateDeadZone0_1(data, dz, data <= dz);

		return 1;
	}

	void Gamepad::_updateStick(LONG& oriX, LONG& oriY, LONG curX, LONG curY, GamepadKeyCode key) {
		f32 value[] = { _translateStick(curX) , _translateStick(curY) };
		auto dz = _getDeadZone(key);
		auto oriXDz = Math::isEqual(_translateStick(oriX), 0.f, dz);
		auto oriYDz = Math::isEqual(_translateStick(oriY), 0.f, dz);
		auto curXDz = Math::isEqual(value[0], 0.f, dz);
		auto curYDz = Math::isEqual(value[1], 0.f, dz);
		_translateDeadZone_1_1(value[0], dz, curXDz);
		_translateDeadZone_1_1(value[1], dz, curYDz);
		oriX = curX;
		oriY = curY;
		if (!curXDz || !curYDz || oriXDz != curXDz || oriYDz != curYDz) _eventDispatcher.dispatchEvent(this, InputDeviceEvent::MOVE, &InputKey({ (ui8)key, 2, value }));
	}

	void Gamepad::_updateTrigger(LONG& ori, LONG cur, GamepadKeyCode key) {
		f32 value = _translateTrigger(cur);
		auto dz = _getDeadZone(key);
		auto oriDz = _translateTrigger(ori) <= dz;
		auto curDz = value <= dz;
		_translateDeadZone0_1(value, dz, curDz);
		ori = cur;
		if (!curDz || oriDz != curDz) _eventDispatcher.dispatchEvent(this, InputDeviceEvent::MOVE, &InputKey({ (ui8)key, 1, &value }));
	}
}