#include "GamepadWindows.h"

#if AE_OS == AE_OS_WIN
#include "aurora/HID.h"
#include "aurora/Debug.h"

namespace aurora::modules::inputs::hid_input {
	Gamepad::Gamepad(Input& input, const DeviceInfo& info, extensions::HIDDevice& hid) : GamepadBase(input, info, hid),
		_maxValidAxes(0) {
		using namespace aurora::extensions;
		using namespace aurora::enum_operators;

		_preparsedData = (PHIDP_PREPARSED_DATA)HID::getPreparsedData(*_hid);

		memset(_axisCaps, 0, sizeof(_axisCaps));
		_dpadCap.valid = false;

		HIDP_CAPS caps;
		if (HidP_GetCaps(_preparsedData, &caps) == HIDP_STATUS_SUCCESS) {
			if (caps.NumberInputValueCaps) {
				std::vector<HIDP_VALUE_CAPS> valueCaps;
				valueCaps.resize(caps.NumberInputValueCaps);
				if (HidP_GetValueCaps(HidP_Input, valueCaps.data(), &caps.NumberInputValueCaps, _preparsedData) == HIDP_STATUS_SUCCESS) {
					for (auto& caps : valueCaps) {
						if (caps.UsagePage != HIDReportUsagePageType::GENERIC_DESKTOP) continue;

						auto min = _translateRangeVal(caps.LogicalMin, caps.BitSize), max = _translateRangeVal(caps.LogicalMax, caps.BitSize);

						if (caps.NotRange.Usage == HIDReportGenericDesktopPageType::HAT_SWITCH) {
							_dpadCap.valid = true;
							_dpadCap.min = min;
							_dpadCap.max = max;
						} else {
							if (caps.Range.UsageMin > HIDReportGenericDesktopPageType::RZ || caps.Range.UsageMax < HIDReportGenericDesktopPageType::X) continue;

							if (caps.Range.UsageMin < (USAGE)HIDReportGenericDesktopPageType::X) caps.Range.UsageMin = (USAGE)HIDReportGenericDesktopPageType::X;
							if (caps.Range.UsageMax > (USAGE)HIDReportGenericDesktopPageType::RZ) caps.Range.UsageMax = (USAGE)HIDReportGenericDesktopPageType::RZ;

							for (auto i = caps.Range.UsageMin, n = caps.Range.UsageMax; i <= n; ++i) {
								auto& ac = _axisCaps[i - (USAGE)HIDReportGenericDesktopPageType::X];
								ac.valid = true;
								ac.min = min;
								ac.lengthReciprocal = 1.0f / (max - min);

								++_maxValidAxes;
							}

							_inputValueCaps.emplace_back(caps);
						}
					}
				}
			}
		}
		

		_setKeyMapping(nullptr);
		
		memset(&_inputState, 0, sizeof(_inputState));

		InputBuffer buf;
		do {
			if (auto rst = HID::read(*_hid, buf, sizeof(buf), HID::IN_TIMEOUT_BLOCKING); HID::isSuccess(rst)) {
				_readState(_inputState, buf, rst);
				break;
			} else if (rst == HID::OUT_ERROR) {
				break;
			}
		} while (true);
	}

	DeviceState::CountType Gamepad::getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const {
		using namespace aurora::enum_operators;

		switch (type) {
		case DeviceStateType::KEY:
		{
			if (values && count) {
				std::shared_lock lock(_inputMutex);

				switch ((GamepadVirtualKeyCode)code) {
				case GamepadVirtualKeyCode::L_STICK:
					return _getStick(GamepadVirtualKeyCode::L_STICK_X, (GamepadVirtualKeyCode)code, (DeviceStateValue*)values, count);
				case GamepadVirtualKeyCode::R_STICK:
					return _getStick(GamepadVirtualKeyCode::R_STICK_X, (GamepadVirtualKeyCode)code, (DeviceStateValue*)values, count);
				case GamepadVirtualKeyCode::L_TRIGGER:
				{
					GamepadKeyCode mappingVals[2];
					_keyMapping.get(GamepadVirtualKeyCode::L_TRIGGER, 2, mappingVals);
					return mappingVals[0] == mappingVals[1] ?
						_getCombinedTrigger(mappingVals[0], (GamepadVirtualKeyCode)code, 0, ((DeviceStateValue*)values)[0]) :
						_getAxis(mappingVals[0], (GamepadVirtualKeyCode)code, ((DeviceStateValue*)values)[0]);
				}
				case GamepadVirtualKeyCode::R_TRIGGER:
				{
					GamepadKeyCode mappingVals[2];
					_keyMapping.get(GamepadVirtualKeyCode::L_TRIGGER, 2, mappingVals);
					return mappingVals[0] == mappingVals[1] ?
						_getCombinedTrigger(mappingVals[0], (GamepadVirtualKeyCode)code, 1, ((DeviceStateValue*)values)[0]) :
						_getAxis(mappingVals[1], (GamepadVirtualKeyCode)code, ((DeviceStateValue*)values)[0]);
				}
				case GamepadVirtualKeyCode::DPAD:
					((DeviceStateValue*)values)[0] = _translateDpad(_inputState.dpad, _dpadCap);
					return 1;
				default:
				{
					if (code >= GamepadVirtualKeyCode::SEPARATE_AXIS_START && code <= GamepadVirtualKeyCode::UNDEFINED_AXIS_END) {
						_getAxis(_keyMapping.get((GamepadVirtualKeyCode)code), (GamepadVirtualKeyCode)code, ((DeviceStateValue*)values)[0]);
						return 1;
					} else if (code >= GamepadVirtualKeyCode::BUTTON_START && code <= GamepadVirtualKeyCode::BUTTON_END) {
						((DeviceStateValue*)values)[0] = _translateButton(_readButtonVal(_inputState, _keyMapping.get((GamepadVirtualKeyCode)code)));
						return 1;
					}

					return 0;
				}
				}
			}

			return 0;
		}
		case DeviceStateType::KEY_MAPPING:
		{
			if (values && count) {
				std::shared_lock lock(_inputMutex);

				*((GamepadKeyMapping*)values) = _keyMapping;

				return 1;
			}

			return 0;
		}
		default:
			return GamepadBase::getState(type, code, values, count);
		}
	}

	DeviceState::CountType Gamepad::setState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count) {
		switch (type) {
		case DeviceStateType::KEY_MAPPING:
		{
			if (!count) values = nullptr;

			{
				std::scoped_lock lock(_inputMutex);

				_setKeyMapping((const GamepadKeyMapping*)values);
			}

			return 1;
		}
		default:
			return GamepadBase::setState(type, code, values, count);
		}
	}

	void Gamepad::poll(bool dispatchEvent) {
		using namespace aurora::enum_operators;
		using namespace aurora::extensions;

		InputBuffer inputBuffer;
		auto inputBufferSize = HID::read(*_hid, inputBuffer, sizeof(InputBuffer), 0);
		if (HID::isSuccess(inputBufferSize)) {
			InputState newState;
			_readState(newState, inputBuffer, inputBufferSize);

			if (!dispatchEvent) {
				std::scoped_lock lock(_inputMutex);

				memcpy(&_inputState, &newState, sizeof(_inputState));

				return;
			}

			InputState oldState;
			GamepadKeyMapping keyMapping(NO_INIT);
			{
				std::scoped_lock lock(_inputMutex);

				keyMapping = _keyMapping;
				memcpy(&oldState, &_inputState, sizeof(_inputState));
				memcpy(&_inputState, &newState, sizeof(_inputState));
			}

			GamepadKeyCode mappingVals[4];
			keyMapping.get(GamepadVirtualKeyCode::L_STICK_X, 4, mappingVals);
			for (size_t i = 0; i < 2; ++i) {
				auto idx = i << 1;
				_dispatchStick(
					_readAxisVal(oldState, mappingVals[idx], 0.5f),
					_readAxisVal(oldState, mappingVals[idx + 1], 0.5f),
					_readAxisVal(newState, mappingVals[idx], 0.5f),
					_readAxisVal(newState, mappingVals[idx + 1], 0.5f),
					GamepadVirtualKeyCode::L_STICK + i);
			}

			keyMapping.get(GamepadVirtualKeyCode::L_TRIGGER, 2, mappingVals);
			if (mappingVals[0] == mappingVals[1]) {
				_dispatchCombinedTrigger(_readAxisVal(oldState, mappingVals[0], 0.5f), _readAxisVal(newState, mappingVals[1], 0.5f));
			} else {
				for (size_t i = 0; i < 2; ++i) _dispatchAxis(_readAxisVal(oldState, mappingVals[i], 0.0f), _readAxisVal(newState, mappingVals[i], 0.0f), GamepadVirtualKeyCode::L_TRIGGER + i);
			}

			keyMapping.forEach([&](GamepadVirtualKeyCode vk, GamepadKeyCode k) {
				if (vk >= GamepadVirtualKeyCode::UNDEFINED_AXIS_1 && vk <= GamepadVirtualKeyCode::AXIS_END) {
					_dispatchAxis(_readAxisVal(oldState, k, 0.0f), _readAxisVal(newState, k, 0.0f), vk);
				} else if (vk >= GamepadVirtualKeyCode::BUTTON_START && vk <= GamepadVirtualKeyCode::BUTTON_END) {
					if (auto newVal = _readButtonVal(newState, k); newVal != _readButtonVal(oldState, k)) {
						auto value = _translateButton(newVal);
						DeviceState ds = { (DeviceState::CodeType)vk, 1, &value };
						_eventDispatcher->dispatchEvent(this, value > Math::ZERO<DeviceStateValue> ? DeviceEvent::DOWN : DeviceEvent::UP, &ds);
					}
				}
			});

			_dispatchDpad(oldState.dpad, newState.dpad);
		}

		int a = 1;
	}

	void Gamepad::_setKeyMapping(const GamepadKeyMapping* mapping) {
		using namespace aurora::enum_operators;
		using namespace aurora::extensions;

		if (mapping) {
			_keyMapping = *mapping;
		} else {
			_keyMapping.clear();

			bool usedAxes[MAX_AXES];
			memset(usedAxes, 1, sizeof(usedAxes));

			for (size_t i = 0; i < MAX_AXES; ++i) {
				auto& ac = _axisCaps[i];
				if (ac.valid) usedAxes[i] = false;
			}

			auto getAndSet = [&]() {
				for (size_t i = 0; i < MAX_AXES; ++i) {
					if (!usedAxes[i]) {
						usedAxes[i] = true;
						return i;
					}
				}
				return Math::ZERO<size_t>;
			};

			size_t lastAxes = _maxValidAxes;
			for (size_t i = 0; i < 2; ++i) {
				if (lastAxes >= 2) {
					auto setStick = false;
					auto sx = GamepadVirtualKeyCode::L_STICK_X + (i << 1);

					if (!usedAxes[0] && !usedAxes[1]) {
						_keyMapping.set(sx, GamepadKeyCode::AXIS_1);
						_keyMapping.set(sx + 1, GamepadKeyCode::AXIS_1 + 1);

						setStick = true;
						usedAxes[0] = true;
						usedAxes[1] = true;
						lastAxes -= 2;
					}
					if (!setStick && !usedAxes[3] && !usedAxes[4]) {
						_keyMapping.set(sx, GamepadKeyCode::AXIS_1 + 3);
						_keyMapping.set(sx + 1, GamepadKeyCode::AXIS_1 + 4);

						setStick = true;
						usedAxes[3] = true;
						usedAxes[4] = true;
						lastAxes -= 2;
					}
					if (!setStick && !usedAxes[2] && !usedAxes[5]) {
						_keyMapping.set(sx, GamepadKeyCode::AXIS_1 + 2);
						_keyMapping.set(sx + 1, GamepadKeyCode::AXIS_1 + 5);

						setStick = true;
						usedAxes[2] = true;
						usedAxes[5] = true;
						lastAxes -= 2;
					}
					if (!setStick) {
						_keyMapping.set(sx, GamepadKeyCode::AXIS_1 + getAndSet());
						_keyMapping.set(sx + 1, GamepadKeyCode::AXIS_1 + getAndSet());

						lastAxes -= 2;
					}
				}
			}

			if (lastAxes >= 1) {
				if (lastAxes == 1) {
					auto i = getAndSet();
					_keyMapping.set(GamepadVirtualKeyCode::L_TRIGGER, GamepadKeyCode::AXIS_1 + i);
					_keyMapping.set(GamepadVirtualKeyCode::R_TRIGGER, GamepadKeyCode::AXIS_1 + i);
				} else {
					_keyMapping.set(GamepadVirtualKeyCode::L_TRIGGER, GamepadKeyCode::AXIS_1 + getAndSet());
					_keyMapping.set(GamepadVirtualKeyCode::R_TRIGGER, GamepadKeyCode::AXIS_1 + getAndSet());
				}
			}

			_keyMapping.set(GamepadVirtualKeyCode::A, GamepadKeyCode::BUTTON_1);
			_keyMapping.set(GamepadVirtualKeyCode::B, GamepadKeyCode::BUTTON_1 + 1);
			_keyMapping.set(GamepadVirtualKeyCode::X, GamepadKeyCode::BUTTON_1 + 2);
			_keyMapping.set(GamepadVirtualKeyCode::Y, GamepadKeyCode::BUTTON_1 + 3);
			_keyMapping.set(GamepadVirtualKeyCode::L_SHOULDER, GamepadKeyCode::BUTTON_1 + 4);
			_keyMapping.set(GamepadVirtualKeyCode::R_SHOULDER, GamepadKeyCode::BUTTON_1 + 5);
			_keyMapping.set(GamepadVirtualKeyCode::BACK, GamepadKeyCode::BUTTON_1 + 6);
			_keyMapping.set(GamepadVirtualKeyCode::START, GamepadKeyCode::BUTTON_1 + 7);
			_keyMapping.set(GamepadVirtualKeyCode::L_THUMB, GamepadKeyCode::BUTTON_1 + 8);
			_keyMapping.set(GamepadVirtualKeyCode::R_THUMB, GamepadKeyCode::BUTTON_1 + 9);
		}

		_keyMapping.undefinedCompletion(_maxValidAxes, MAX_BUTTONS);
	}

	void Gamepad::_readState(InputState& state, const uint8_t* inputBuffer, size_t inputBufferSize) {
		using namespace aurora::extensions;
		using namespace aurora::enum_operators;

		state.buttons = 0;

		USAGE usages[MAX_BUTTONS];
		ULONG numPressed = MAX_BUTTONS;
		if (HidP_GetUsages(HidP_Input, (USAGE)HIDReportUsagePageType::BUTTON, 0, usages, &numPressed, _preparsedData, (PCHAR)inputBuffer, inputBufferSize) == HIDP_STATUS_SUCCESS) {
			for (size_t i = 0; i < numPressed; ++i) {
				auto usage = usages[i];
				if (usage >= HIDReportButtonPageType::BUTTON_1 && usage <= MAX_HID_BUTTON_PAGE) state.buttons |= Math::ONE<uint_t<MAX_BUTTONS>> << (usage - (USHORT)HIDReportButtonPageType::BUTTON_1);
			}
		}

		for (auto& caps : _inputValueCaps) {
			for (auto i = caps.Range.UsageMin; i <= caps.Range.UsageMax; ++i) {
				ULONG val = 0;
				HidP_GetUsageValue(HidP_Input, caps.UsagePage, 0, i, &val, _preparsedData, (PCHAR)inputBuffer, inputBufferSize);

				auto idx = i - (size_t)HIDReportGenericDesktopPageType::X;
				auto& ac = _axisCaps[idx];
				state.axes[idx] = (val - ac.min) * ac.lengthReciprocal;
			}
		}

		ULONG val = 0;
		if (HidP_GetUsageValue(HidP_Input, (USAGE)HIDReportUsagePageType::GENERIC_DESKTOP, 0, (USAGE)HIDReportGenericDesktopPageType::HAT_SWITCH, &val, _preparsedData, (PCHAR)inputBuffer, inputBufferSize) == HIDP_STATUS_SUCCESS) {
			state.dpad = val;
		} else {
			state.dpad = 0;
		}
	}

	DeviceState::CountType Gamepad::_getStick(GamepadVirtualKeyCode stickX, GamepadVirtualKeyCode key, DeviceStateValue* data, DeviceState::CountType count) const {
		GamepadKeyCode mappingVals[2];
		_keyMapping.get(stickX, 2, mappingVals);

		return translate(
			_normalizeStick(_readAxisVal(_inputState, mappingVals[0], 0.5f)),
			_normalizeStick(_readAxisVal(_inputState, mappingVals[1], 0.5f)),
			_getDeadZone(key), data, count);
	}

	DeviceState::CountType Gamepad::_getCombinedTrigger(GamepadKeyCode k, GamepadVirtualKeyCode vk, uint8_t index, DeviceStateValue& data) const {
		DeviceStateValue values[2];
		_normalizeCombinedAxis(_readAxisVal(_inputState, k, 0.5f), values[1], values[0]);
		data = translate(values[index], _getDeadZone(vk));

		return 1;
	}

	DeviceState::CountType Gamepad::_getAxis(GamepadKeyCode k, GamepadVirtualKeyCode vk, DeviceStateValue& data) const {
		data = translate(_readAxisVal(_inputState, k, 0.0f), _getDeadZone(vk));
		return 1;
	}

	void Gamepad::_dispatchStick(float32_t oldX, float32_t oldY, float32_t newX, float32_t newY, GamepadVirtualKeyCode key) {
		auto dz = _getDeadZone(key);

		DeviceStateValue oldDzVals[2], newDzVals[2];
		translate(_normalizeStick(oldX), _normalizeStick(oldY), dz, oldDzVals, 2);
		translate(_normalizeStick(newX), _normalizeStick(newY), dz, newDzVals, 2);

		if (oldDzVals[0] != newDzVals[0] || oldDzVals[1] != newDzVals[1]) {
			DeviceState ds = { (DeviceState::CodeType)key, 2, newDzVals };
			_eventDispatcher->dispatchEvent((void*)this, DeviceEvent::MOVE, &ds);
		}
	}

	void Gamepad::_dispatchCombinedTrigger(float32_t oldVal, float32_t newVal) {
		DeviceStateValue oldVals[2], newVals[2];
		_normalizeCombinedAxis(oldVal, oldVals[1], oldVals[0]);
		_normalizeCombinedAxis(newVal, newVals[1], newVals[0]);
		auto ldz = _getDeadZone(GamepadVirtualKeyCode::L_TRIGGER);
		auto rdz = _getDeadZone(GamepadVirtualKeyCode::R_TRIGGER);

		if (auto newDzVal = translate(newVals[0], ldz); newDzVal != translate(oldVals[0], ldz)) {
			DeviceState ds = { (DeviceState::CodeType)GamepadVirtualKeyCode::L_TRIGGER, 1, &newDzVal };
			_eventDispatcher->dispatchEvent((void*)this, DeviceEvent::MOVE, &ds);
		}

		if (auto newDzVal = translate(newVals[1], rdz); newDzVal != translate(oldVals[1], rdz)) {
			DeviceState ds = { (DeviceState::CodeType)GamepadVirtualKeyCode::R_TRIGGER, 1, &newDzVal };
			_eventDispatcher->dispatchEvent((void*)this, DeviceEvent::MOVE, &ds);
		}
	}

	void Gamepad::_dispatchAxis(float32_t oldVal, float32_t newVal, GamepadVirtualKeyCode key) {
		auto dz = _getDeadZone(key);
		if (auto newDzVal = translate(newVal, dz); newDzVal != translate(oldVal, dz)) {
			DeviceState ds = { (DeviceState::CodeType)key, 1, &newDzVal };
			_eventDispatcher->dispatchEvent((void*)this, DeviceEvent::MOVE, &ds);
		}
	}

	void Gamepad::_dispatchDpad(uint32_t oldVal, uint32_t newVal) {
		if (oldVal != newVal) {
			if (auto val = _translateDpad(newVal, _dpadCap); val != _translateDpad(oldVal, _dpadCap)) {
				DeviceState k = { (DeviceState::CodeType)GamepadVirtualKeyCode::DPAD, 1, &val };
				_eventDispatcher->dispatchEvent(this, val < Math::ZERO<DeviceStateValue> ? DeviceEvent::UP : DeviceEvent::DOWN, &k);
			}
		}
	}

	uint32_t Gamepad::_translateRangeVal(LONG rawVal, size_t numBits) {
		uint32_t val = 0;
		for (size_t i = 0; i < numBits; ++i) val |= 1 << i;
		val &= (std::make_unsigned_t<decltype(val)>)rawVal;
		return val;
	}

	DeviceStateValue Gamepad::_translateDpad(uint32_t val, const DPadCap& cap) {
		if (cap.valid && val >= cap.min && val <= cap.max) {
			if (cap.max - cap.min + 1 == 8) {
				return (val - cap.min) * Math::PI_8<DeviceStateValue>;
			}
		}

		return Math::NEGATIVE_ONE<DeviceStateValue>;
	}
}
#endif