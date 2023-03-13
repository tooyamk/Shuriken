#include "GamepadDriverWindows.h"
#include "srk/Debug.h"

#if SRK_OS == SRK_OS_WINDOWS
#include "Input.h"

namespace srk::modules::inputs::hid_input {
	GamepadDriver::GamepadDriver(Input& input, extensions::HIDDevice& hid, PHIDP_PREPARSED_DATA preparsedData, DeviceDesc&& desc) : GamepadDriverBase(input, hid),
		_preparsedData(preparsedData),
		_desc(std::move(desc)) {
		using namespace srk::enum_operators;

		_offset.axis = HEADER_LENGTH + _desc.inputReportByteLength;
		auto axisLength = _desc.axes.size() * sizeof(float32_t);
		_offset.dpad = _offset.axis + axisLength;
		auto dpadLength = _desc.dpad.valid ? sizeof(uint32_t) : 0;
		auto buttonUsagesLength = sizeof(USAGE) * _desc.buttons.size();
		size_t paddingLength = 0;
		if (axisLength + dpadLength < buttonUsagesLength) paddingLength = buttonUsagesLength - axisLength - dpadLength;
		_offset.button = _offset.dpad + dpadLength + paddingLength;

		_maxAxisKeyCode = GamepadKeyCode::AXIS_1 + _desc.axes.size();
		if (_desc.dpad.valid) _maxAxisKeyCode = _maxAxisKeyCode + 2;
		_maxButtonKeyCode = GamepadKeyCode::BUTTON_1 + _desc.buttons.size();
	}

	GamepadDriver::~GamepadDriver() {
	}

	GamepadDriver* GamepadDriver::create(Input& input, extensions::HIDDevice& hid) {
		using namespace srk::extensions;
		using namespace srk::enum_operators;

		auto preparsedData = (PHIDP_PREPARSED_DATA)HID::getPreparsedData(hid);
		if (!preparsedData) return nullptr;

		DeviceDesc desc;
		desc.dpad.valid = false;

		HIDP_CAPS caps;
		if (HidP_GetCaps(preparsedData, &caps) != HIDP_STATUS_SUCCESS) return nullptr;
			
		if (caps.NumberInputValueCaps) {
			std::vector<HIDP_VALUE_CAPS> valueCaps(caps.NumberInputValueCaps);
			if (HidP_GetValueCaps(HidP_Input, valueCaps.data(), &caps.NumberInputValueCaps, preparsedData) == HIDP_STATUS_SUCCESS) {
				for (auto& caps : valueCaps) {
					if (caps.UsagePage != HIDReportUsagePageType::GENERIC_DESKTOP) continue;

					auto min = _translateRangeVal(caps.LogicalMin, caps.BitSize), max = _translateRangeVal(caps.LogicalMax, caps.BitSize);

					if (caps.NotRange.Usage == HIDReportGenericDesktopPageType::HAT_SWITCH) {
						desc.dpad.valid = true;
						desc.dpad.min = min;
						desc.dpad.max = max;
					} else {
						if (caps.Range.UsageMin > HIDReportGenericDesktopPageType::RZ || caps.Range.UsageMax < HIDReportGenericDesktopPageType::X) continue;

						if (caps.Range.UsageMin < (USAGE)HIDReportGenericDesktopPageType::X) caps.Range.UsageMin = (USAGE)HIDReportGenericDesktopPageType::X;
						if (caps.Range.UsageMax > (USAGE)HIDReportGenericDesktopPageType::RZ) caps.Range.UsageMax = (USAGE)HIDReportGenericDesktopPageType::RZ;

						for (auto i = caps.Range.UsageMin, n = caps.Range.UsageMax; i <= n; ++i) {
							auto& axis = desc.axes.emplace_back();
							axis.usagePage = (HIDReportUsagePageType)caps.UsagePage;
							axis.usage = (HIDReportGenericDesktopPageType)i;
							axis.min = min;
							axis.lengthReciprocal = 1.0f / (max - min);
						}
					}
				}
			}
		}

		if (caps.NumberInputButtonCaps) {
			std::vector<HIDP_BUTTON_CAPS> btnCaps(caps.NumberInputButtonCaps);
			if (HidP_GetButtonCaps(HidP_Input, btnCaps.data(), &caps.NumberInputButtonCaps, preparsedData) == HIDP_STATUS_SUCCESS) {
				for (auto& cap : btnCaps) {
					if (cap.UsagePage != HIDReportUsagePageType::BUTTON) continue;

					if (cap.IsRange) {
						for (auto i = cap.Range.UsageMin, n = cap.Range.UsageMax; i <= n; ++i) desc.buttons.emplace_back((HIDReportButtonPageType)i);
					} else {
						desc.buttons.emplace_back((HIDReportButtonPageType)cap.NotRange.Usage);
					}
				}
			}
		}

		std::sort(desc.axes.begin(), desc.axes.end(), [](const DeviceDesc::Axis& l, const DeviceDesc::Axis& r) {
			return l.usage < r.usage;
		});
		std::sort(desc.buttons.begin(), desc.buttons.end(), [](HIDReportButtonPageType l, HIDReportButtonPageType r) {
			return l < r;
		});

		for (size_t i = 0, n = desc.buttons.size(); i < n; ++i) desc.buttonMapper.emplace(desc.buttons[i], i);

		desc.inputReportByteLength = caps.InputReportByteLength;
		desc.outputReportByteLength = caps.OutputReportByteLength;
		desc.dpad.unitAngle = Math::PI2<DeviceStateValue> / (desc.dpad.max - desc.dpad.min + 1);

		return new GamepadDriver(input, hid, preparsedData, std::move(desc));
	}

	size_t GamepadDriver::getInputLength() const {
		return _offset.button + _desc.buttons.size();
	}

	size_t GamepadDriver::getOutputLength() const {
		return 0;
	}

	bool GamepadDriver::init(void* inputState, void* outputState) {
		if (inputState) ((uint8_t*)inputState)[0] = 0;

		return true;
	}

	bool GamepadDriver::isStateReady(const void* state) const {
		return ((const uint8_t*)state)[0];
	}

	bool GamepadDriver::readStateFromDevice(void* inputState) const {
		using namespace srk::extensions;

		auto buffer = (uint8_t*)inputState;
		auto inputReportData = (PCHAR)(buffer + HEADER_LENGTH);
		if (auto rst = HID::read(*_hid, inputReportData, _desc.inputReportByteLength, 0); HID::isSuccess(rst)) {
			if (!_desc.buttons.empty()) {
				auto buttonState = (bool*)(buffer + _offset.button);
				memset(buttonState, 0, _desc.buttons.size());

				ULONG numPressed = _desc.buttons.size();
				auto usages = (USAGE*)(buffer + _offset.axis);
				if (HidP_GetUsages(HidP_Input, (USAGE)HIDReportUsagePageType::BUTTON, 0, usages, &numPressed, _preparsedData, inputReportData, rst) == HIDP_STATUS_SUCCESS) {
					for (decltype(numPressed) i = 0; i < numPressed; ++i) {
						auto usage = (HIDReportButtonPageType)usages[i];
						if (auto itr = _desc.buttonMapper.find(usage); itr != _desc.buttonMapper.end()) buttonState[itr->second] = true;
					}
				}
			}

			if (!_desc.axes.empty()) {
				auto axisState = (float32_t*)(buffer + _offset.axis);
				for (size_t i = 0, n = _desc.axes.size(); i < n; ++i) {
					const auto& axis = _desc.axes[i];
					ULONG val = 0;
					if (HidP_GetUsageValue(HidP_Input, (USAGE)axis.usagePage, 0, (USAGE)axis.usage, &val, _preparsedData, inputReportData, rst) == HIDP_STATUS_SUCCESS) {
						axisState[i] = (val - axis.min) * axis.lengthReciprocal;
					} else {
						axisState[i] = 0.f;
					}
				}
			}

			if (_desc.dpad.valid) {
				auto dpadState = (uint32_t*)(buffer + _offset.dpad);
				ULONG val = 0;
				if (HidP_GetUsageValue(HidP_Input, (USAGE)HIDReportUsagePageType::GENERIC_DESKTOP, 0, (USAGE)HIDReportGenericDesktopPageType::HAT_SWITCH, &val, _preparsedData, inputReportData, rst) == HIDP_STATUS_SUCCESS) {
					dpadState[0] = val;
				} else {
					dpadState[0] = 0;
				}
			}

			buffer[0] = 1;

			return true;
		}

		return true;
	}

	float32_t GamepadDriver::readDataFromInputState(const void* inputState, GamepadKeyCodeAndFlags cf, float32_t defaultVal) const {
		using namespace srk::enum_operators;

		float32_t val;
		if (auto raw = (const uint8_t*)inputState; raw[0]) {
			if (cf.code >= GamepadKeyCode::AXIS_1 && cf.code <= _maxAxisKeyCode) {
				if (_desc.dpad.valid && cf.code >= _maxAxisKeyCode - 2) {
					auto dpadVal = *(uint32_t*)(raw + _offset.dpad);
					if (dpadVal >= _desc.dpad.min && dpadVal <= _desc.dpad.max) {
						auto a = (dpadVal - _desc.dpad.min) * _desc.dpad.unitAngle;
						val = 0.5f + (cf.code == _maxAxisKeyCode - 1 ? std::sin(a) : std::cos(a)) * 0.5f;
					} else {
						val = 0.5f;
					}
				} else {
					val = ((float32_t*)(raw + _offset.axis))[(size_t)(cf.code - GamepadKeyCode::AXIS_1)];
				}
			} else if (cf.code >= GamepadKeyCode::BUTTON_1 && cf.code <= _maxButtonKeyCode) {
				val = ((bool*)(raw + _offset.button))[(size_t)(cf.code - GamepadKeyCode::BUTTON_1)] ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;
			} else {
				val = defaultVal;
			}
		} else {
			val = defaultVal;
		}

		return translate(val, cf.flags);
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

	void GamepadDriver::setKeyMapper(GamepadKeyMapper& dst, const GamepadKeyMapper* src) const {
		using namespace srk::enum_operators;

		if (src) {
			dst = *src;
		} else {
			dst.setDefault(_desc.axes.size(), _desc.dpad.valid ? 1 : 0, _desc.buttons.size(), false);
		}

		dst.undefinedCompletion(_desc.axes.size() + (_desc.dpad.valid ? 2 : 0), _desc.buttons.size());
	}

	uint32_t GamepadDriver::_translateRangeVal(LONG rawVal, size_t numBits) {
		uint32_t val = 0;
		for (size_t i = 0; i < numBits; ++i) val |= 1 << i;
		val &= (std::make_unsigned_t<decltype(val)>)rawVal;
		return val;
	}
}
#endif