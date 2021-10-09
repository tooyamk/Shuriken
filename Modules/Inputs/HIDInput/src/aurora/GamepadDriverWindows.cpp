#include "GamepadDriverWindows.h"
#include "aurora/Debug.h"

#if AE_OS == AE_OS_WINDOWS
#include "Input.h"

namespace aurora::modules::inputs::hid_input {
	GamepadDriver::GamepadDriver(Input& input, extensions::HIDDevice& hid) : GamepadDriverBase(input, hid),
		_preparsedData(nullptr),
		_maxValidAxes(0) {
		memset(_axisCaps, 0, sizeof(_axisCaps));
		_dpadCap.valid = false;
	}

	GamepadDriver::~GamepadDriver() {
	}

	size_t GamepadDriver::getInputLength() const {
		return sizeof(InputState) + 1;
	}

	size_t GamepadDriver::getOutputLength() const {
		return 0;
	}

	bool GamepadDriver::init(void* inputState, void* outputState) {
		using namespace aurora::extensions;
		using namespace aurora::enum_operators;

		_preparsedData = (PHIDP_PREPARSED_DATA)HID::getPreparsedData(*_hid);
		_inputValueCaps.clear();
		_maxValidAxes = 0;

		memset(_axisCaps, 0, sizeof(_axisCaps));
		_dpadCap.valid = false;

		HIDP_CAPS caps;
		if (HidP_GetCaps(_preparsedData, &caps) == HIDP_STATUS_SUCCESS) {
			if (caps.NumberInputValueCaps) {
				std::vector<HIDP_VALUE_CAPS> valueCaps(caps.NumberInputValueCaps);
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

		((uint8_t*)inputState)[0] = false;

		return true;
	}

	bool GamepadDriver::isStateReady(const void* state) const {
		return ((const uint8_t*)state)[0];
	}

	bool GamepadDriver::readStateFromDevice(void* inputState) const {
		using namespace aurora::extensions;

		uint8_t buf[128];
		if (auto rst = HID::read(*_hid, buf, sizeof(buf), 0); HID::isSuccess(rst)) {
			auto raw = (uint8_t*)inputState;
			_parseInputState(*(InputState*)(raw + 1), buf, rst);
			raw[0] = 1;

			return true;
		}

		return true;
	}

	float32_t GamepadDriver::readDataFromInputState(const void* inputState, GamepadKeyCodeAndFlags cf, float32_t defaultVal) const {
		using namespace aurora::enum_operators;

		float32_t val;
		if (auto raw = (const uint8_t*)inputState; raw[0]) {
			auto data = (const InputState*)(raw + 1);

			if (cf.code >= GamepadKeyCode::AXIS_1 && cf.code <= MAX_AXIS_KEY) {
				val = data->axes[(size_t)(cf.code - GamepadKeyCode::AXIS_1)];
			} else if (cf.code >= GamepadKeyCode::BUTTON_1 && cf.code <= MAX_BUTTON_KEY) {
				val = data->buttons & (Math::ONE<uint_t<MAX_BUTTONS>> << (size_t)(cf.code - GamepadKeyCode::BUTTON_1)) ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;
			} else {
				val = defaultVal;
			}
		} else {
			val = defaultVal;
		}

		return translate(val, cf.flags);
	}

	float32_t GamepadDriver::readDpadDataFromInputState(const void* inputState) const {
		if (auto raw = (const uint8_t*)inputState; raw[0]) {
			auto data = (const InputState*)(raw + 1);

			if (_dpadCap.valid && data->dpad >= _dpadCap.min && data->dpad <= _dpadCap.max) {
				if (_dpadCap.max - _dpadCap.min + 1 == 8) {
					return (data->dpad - _dpadCap.min) * Math::PI_8<DeviceStateValue>;
				}
			}
		}

		return Math::NEGATIVE_ONE<DeviceStateValue>;
	}

	DeviceState::CountType GamepadDriver::customGetState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count,
		const void* inputState, void* custom, ReadWriteStateStartCallback readStateStartCallback, ReadWriteStateStartCallback readStateEndCallback) const {
		return 0;
	}

	void GamepadDriver::customDispatch(const void* oldInputState, const void* newInputState, void* custom, DispatchCallback dispatchCallback) const {
	}

	bool GamepadDriver::writeStateToDevice(const void* outputState) const {
		return false;
	}

	DeviceState::CountType GamepadDriver::customSetState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count, void* outputState, void* custom,
		ReadWriteStateStartCallback writeStateStartCallback, ReadWriteStateStartCallback writeStateEndCallback) const {
		return 0;
	}

	void GamepadDriver::setKeyMapping(GamepadKeyMapping& dst, const GamepadKeyMapping* src) const {
		using namespace aurora::enum_operators;

		if (src) {
			dst = *src;
		} else {
			dst.clear();

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
					auto sx = GamepadVirtualKeyCode::L_STICK_X + (i << 1);

					if (!usedAxes[0] && !usedAxes[1]) {
						dst.set(sx, GamepadKeyCode::AXIS_1);
						dst.set(sx + 1, GamepadKeyCode::AXIS_1 + 1);

						usedAxes[0] = true;
						usedAxes[1] = true;
						lastAxes -= 2;
						continue;
					}
					if (!usedAxes[3] && !usedAxes[4]) {
						dst.set(sx, GamepadKeyCode::AXIS_1 + 3);
						dst.set(sx + 1, GamepadKeyCode::AXIS_1 + 4);

						usedAxes[3] = true;
						usedAxes[4] = true;
						lastAxes -= 2;
						continue;
					}
					if (!usedAxes[2] && !usedAxes[5]) {
						dst.set(sx, GamepadKeyCode::AXIS_1 + 2);
						dst.set(sx + 1, GamepadKeyCode::AXIS_1 + 5);

						usedAxes[2] = true;
						usedAxes[5] = true;
						lastAxes -= 2;
						continue;
					}
					{
						dst.set(sx, GamepadKeyCode::AXIS_1 + getAndSet());
						dst.set(sx + 1, GamepadKeyCode::AXIS_1 + getAndSet());

						lastAxes -= 2;
					}
				}
			}

			if (lastAxes >= 1) {
				if (lastAxes == 1) {
					auto i = getAndSet();
					dst.set(GamepadVirtualKeyCode::L_TRIGGER, GamepadKeyCode::AXIS_1 + i, GamepadKeyFlag::HALF_BIG);
					dst.set(GamepadVirtualKeyCode::R_TRIGGER, GamepadKeyCode::AXIS_1 + i, GamepadKeyFlag::FLIP | GamepadKeyFlag::HALF_SMALL);
				} else {
					dst.set(GamepadVirtualKeyCode::L_TRIGGER, GamepadKeyCode::AXIS_1 + getAndSet());
					dst.set(GamepadVirtualKeyCode::R_TRIGGER, GamepadKeyCode::AXIS_1 + getAndSet());
				}
			}

			dst.set(GamepadVirtualKeyCode::A, GamepadKeyCode::BUTTON_1);
			dst.set(GamepadVirtualKeyCode::B, GamepadKeyCode::BUTTON_1 + 1);
			dst.set(GamepadVirtualKeyCode::X, GamepadKeyCode::BUTTON_1 + 2);
			dst.set(GamepadVirtualKeyCode::Y, GamepadKeyCode::BUTTON_1 + 3);
			dst.set(GamepadVirtualKeyCode::L_SHOULDER, GamepadKeyCode::BUTTON_1 + 4);
			dst.set(GamepadVirtualKeyCode::R_SHOULDER, GamepadKeyCode::BUTTON_1 + 5);
			dst.set(GamepadVirtualKeyCode::BACK, GamepadKeyCode::BUTTON_1 + 6);
			dst.set(GamepadVirtualKeyCode::START, GamepadKeyCode::BUTTON_1 + 7);
			dst.set(GamepadVirtualKeyCode::L_THUMB, GamepadKeyCode::BUTTON_1 + 8);
			dst.set(GamepadVirtualKeyCode::R_THUMB, GamepadKeyCode::BUTTON_1 + 9);
		}

		dst.undefinedCompletion(MAX_AXES, MAX_BUTTONS);
	}

	uint32_t GamepadDriver::_translateRangeVal(LONG rawVal, size_t numBits) {
		uint32_t val = 0;
		for (size_t i = 0; i < numBits; ++i) val |= 1 << i;
		val &= (std::make_unsigned_t<decltype(val)>)rawVal;
		return val;
	}

	void GamepadDriver::_parseInputState(InputState& state, const void* inputBuffer, size_t inputBufferSize) const {
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
}
#endif