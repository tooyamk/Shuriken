#include "GamepadWindows.h"

#if AE_OS == AE_OS_WIN
#include "aurora/HID.h"
#include "aurora/Debug.h"

namespace aurora::modules::inputs::hid_input {
	Gamepad::Gamepad(Input& input, const DeviceInfo& info, extensions::HIDDevice& hid) : GamepadBase(input, info, hid) {
		using namespace aurora::extensions;

		_preparsedData = (PHIDP_PREPARSED_DATA)HID::getPreparsedData(*_hid);

		HIDP_CAPS caps;
		if (HidP_GetCaps(_preparsedData, &caps) == HIDP_STATUS_SUCCESS) {
			if (caps.NumberInputButtonCaps) {
				_buttonCaps.resize(caps.NumberInputButtonCaps);
				if (HidP_GetButtonCaps(HidP_Input, _buttonCaps.data(), &caps.NumberInputButtonCaps, _preparsedData) != HIDP_STATUS_SUCCESS) _buttonCaps.clear();
			}

			if (caps.NumberInputValueCaps) {
				_valueCaps.resize(caps.NumberInputValueCaps);
				if (HidP_GetValueCaps(HidP_Input, _valueCaps.data(), &caps.NumberInputValueCaps, _preparsedData) != HIDP_STATUS_SUCCESS) _valueCaps.clear();
			}

			for (auto& caps : _buttonCaps) {
				for (auto i = caps.Range.UsageMin; i <= caps.Range.UsageMax; ++i) {
					auto key = caps.UsagePage << 16 | i;

					_inputState.emplace(key, 0);
					_inputCaps.emplace(key, &caps);
				}
			}

			for (auto& caps : _valueCaps) {
				for (auto i = caps.Range.UsageMin; i <= caps.Range.UsageMax; ++i) {
					auto key = caps.UsagePage << 16 | i;

					_inputState.emplace(key, 0);
					_inputCaps.emplace(key, &caps);
				}
			}
		}
	}

	DeviceState::CountType Gamepad::getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const {
		switch (type) {
		default:
			return GamepadBaseType::getState(type, code, values, count);
		}
	}

	DeviceState::CountType Gamepad::setState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) {
		switch (type) {
		default:
			return GamepadBaseType::setState(type, code, values, count);
		}
	}

	void Gamepad::_doInput(bool dispatchEvent, InputBuffer& inputBuffer, size_t inputBufferSize) {
		using namespace aurora::enum_operators;
		using namespace aurora::extensions;

		std::unordered_map<uint32_t, uint32_t> changed;

		USAGE usages[MAX_BUTTON];
		bool oldBtnState[MAX_BUTTON];
		bool newBtnState[MAX_BUTTON];

		{
			std::scoped_lock lock(_inputStateMutex);

			for (auto& caps : _buttonCaps) {
				ULONG size = MAX_BUTTON;
				if (HidP_GetUsages(HidP_Input, caps.UsagePage, 0, usages, &size, _preparsedData, (PCHAR)inputBuffer, inputBufferSize) == HIDP_STATUS_SUCCESS) {

					for (auto i = caps.Range.UsageMin; i <= caps.Range.UsageMax; ++i) oldBtnState[i - caps.Range.UsageMin] = _inputState[caps.UsagePage << 16 | i];
					memset(newBtnState, 0, size);
					for (decltype(size) i = 0; i < size; ++i) newBtnState[usages[i] - caps.Range.UsageMin] = true;

					for (auto i = caps.Range.UsageMin; i <= caps.Range.UsageMax; ++i) {
						auto idx = i - caps.Range.UsageMin;
						if (oldBtnState[idx] != newBtnState[idx]) {
							auto key = caps.UsagePage << 16 | usages[i];

							_inputState[key] = newBtnState[idx];

							changed.emplace(key, newBtnState[idx] << 16 | oldBtnState[idx]);
						}
					}

					for (auto& caps : _buttonCaps) {
						for (auto i = caps.Range.UsageMin; i <= caps.Range.UsageMax; ++i) _inputState[caps.UsagePage << 16 | i] = 0;
					}

					for (decltype(size) i = 0; i < size; ++i) _inputState[caps.UsagePage << 16 | usages[i]] = 1;
				}
			}

			for (auto& caps : _valueCaps) {
				for (auto i = caps.Range.UsageMin; i <= caps.Range.UsageMax; ++i) {
					ULONG val;
					if (HidP_GetUsageValue(HidP_Input, caps.UsagePage, 0, i, &val, _preparsedData, (PCHAR)inputBuffer, inputBufferSize) == HIDP_STATUS_SUCCESS) {
						auto key = caps.UsagePage << 16 | i;

						if (auto itr = _inputState.find(key); itr->second != val) {
							uint32_t v = val << 16 | itr->second;

							itr->second = val;

							changed.emplace(key, v);
						}
					}
				}
			}
		}

		if (!dispatchEvent) return;

		for (auto& itr : changed) {
			auto usage = itr.first & 0xFFFF;
			auto oldVal = itr.second & 0xFFFF;
			auto newVal = itr.second >> 16;

			switch ((HIDReportUsagePageType)(itr.first >> 16)) {
			case HIDReportUsagePageType::GENERIC_DESKTOP:
			{
				auto& caps = *(HIDP_VALUE_CAPS*)_inputCaps[itr.first];

				switch ((HIDReportGenericDesktopPageType)usage) {
				case HIDReportGenericDesktopPageType::X:
					break;
				case HIDReportGenericDesktopPageType::Y:
					break;
				case HIDReportGenericDesktopPageType::Z:
					break;
				case HIDReportGenericDesktopPageType::RX:
					break;
				case HIDReportGenericDesktopPageType::RY:
					break;
				case HIDReportGenericDesktopPageType::RZ:
					break;
				case HIDReportGenericDesktopPageType::HAT_SWITCH:
				{
					switch (caps.LogicalMax) {
					case 7:
						break;
					case 8:
						break;
					default:
						break;
					}

					break;
				}
				default:
					break;
				}

				break;
			}
			case HIDReportUsagePageType::BUTTON:
			{
				if (usage > HIDReportButtonPageType::NO_BUTTON_PRESED) {
					auto& caps = *(HIDP_BUTTON_CAPS*)_inputCaps[itr.first];
				}

				break;
			}
			default:
				break;
			}
		}

		int a = 1;
	}

	bool Gamepad::_doOutput() {
		return false;
	}
}
#endif