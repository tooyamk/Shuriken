#include "Gamepad.h"

#if AE_OS != AE_OS_WIN
#include "aurora/HID.h"
#include "aurora/Debug.h"

namespace aurora::modules::inputs::hid_input {
	Gamepad::Gamepad(Input& input, const DeviceInfo& info, extensions::HIDDevice& hid) : GamepadBase(input, info, hid) {
	}

	DeviceState::CountType Gamepad::getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const {
		switch (type) {
		default:
			return GamepadBase::getState(type, code, values, count);
		}
	}

	DeviceState::CountType Gamepad::setState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count) {
		switch (type) {
		default:
			return GamepadBase::setState(type, code, values, count);
		}
	}

	/*
	void Gamepad::_parseRawReportDescriptor(ByteArray& raw) {
		using namespace aurora::enum_operators;
		using namespace aurora::extensions;

		if (!raw.isValid()) return;

		HIDReportScopeValues scopeValues;
		
		while (raw.getBytesAvailable()) {
			auto itemHeader = raw.read<uint8_t>();

			auto tag = itemHeader >> 4;
			auto type = itemHeader >> 2 & 0b11;
			auto size = itemHeader & 0b11;

			if (size == 3) size = 4;
			auto val = raw.read<ba_vt::IX>(size);

			switch ((HIDReportItemType)type) {
			case HIDReportItemType::MAIN:
				_parseRawReportItem(scopeValues, (HIDReportMainItemTag)tag, val);
				break;
			case HIDReportItemType::GLOBAL:
				_parseRawReportItem(scopeValues, (HIDReportGlobalItemTag)tag, val);
				break;
			case HIDReportItemType::LOCAL:
				_parseRawReportItem(scopeValues, (HIDReportLocalItemTag)tag, val);
				break;
			default:
				break;
			}
		}

		uint16_t bitIndex = 0;
		for (auto& item : _inputReportItems) {
			auto key = GenericGamepadKeyCode::UNKNOWN;

			switch ((HIDReportUsagePageType)item.usagePage) {
			case HIDReportUsagePageType::GENERIC_DESKTOP:
			{
				switch ((HIDReportGenericDesktopPageType)item.usage) {
				case HIDReportGenericDesktopPageType::X:
					key = GenericGamepadKeyCode::X;
					break;
				case HIDReportGenericDesktopPageType::Y:
					key = GenericGamepadKeyCode::Y;
					break;
				case HIDReportGenericDesktopPageType::Z:
					key = GenericGamepadKeyCode::Z;
					break;
				case HIDReportGenericDesktopPageType::R_X:
					key = GenericGamepadKeyCode::RX;
					break;
				case HIDReportGenericDesktopPageType::R_Y:
					key = GenericGamepadKeyCode::RY;
					break;
				case HIDReportGenericDesktopPageType::R_Z:
					key = GenericGamepadKeyCode::RZ;
					break;
				case HIDReportGenericDesktopPageType::HAT_SWITCH:
					key = GenericGamepadKeyCode::HAT_SWITCH;
					break;
				default:
					break;
				}

				break;
			}
			case HIDReportUsagePageType::BUTTON:
			{
				if (item.usage >= HIDReportButtonPageType::BUTTON_1 && item.usage <= HIDReportButtonPageType::BUTTON_65535) {
					key = GenericGamepadKeyCode::BUTTON_1 + item.usage - (decltype(item.usage))HIDReportButtonPageType::BUTTON_1;
				}

				break;
			}
			default:
				break;
			}

			if (key != GenericGamepadKeyCode::UNKNOWN) {
				for (size_t i = 0; i < item.count; ++i) {
					auto& info = _keyMapping.emplace(std::piecewise_construct, std::forward_as_tuple(key + i), std::forward_as_tuple()).first->second;
					info.logical = item.logical;
					info.bitIndex = bitIndex;
					info.bitCount = item.size;
				}
			}

			bitIndex += item.size * item.count;
		}
	}

	void Gamepad::_parseRawReportItem(aurora::extensions::HIDReportScopeValues& scopeValues, aurora::extensions::HIDReportMainItemTag tag, int32_t val) {
		using namespace aurora::extensions;

		switch (tag) {
		//case HIDReportMainItemTag::COLLECTION:
		//case HIDReportMainItemTag::END_COLLECTION:
		case HIDReportMainItemTag::INPUT:
		{
			auto& item = _inputReportItems.emplace_back();

			if (auto opt = scopeValues.get(HIDReportGlobalItemTag::USAGE_PAGE); opt) item.usagePage = *opt;
			if (auto opt = scopeValues.get(HIDReportLocalItemTag::USAGE); opt) item.usage = *opt;
			if (auto opt = scopeValues.get(HIDReportLocalItemTag::USAGE_MINIMUM); opt) item.usage = *opt;
			if (auto opt = scopeValues.get(HIDReportGlobalItemTag::LOGICAL_MINIMUM); opt) item.logical[0] = *opt;
			if (auto opt = scopeValues.get(HIDReportGlobalItemTag::LOGICAL_MAXIMUM); opt) item.logical[1] = *opt;
			if (auto opt = scopeValues.get(HIDReportGlobalItemTag::PHYSICAL_MINIMUM); opt) item.physical[0] = *opt;
			if (auto opt = scopeValues.get(HIDReportGlobalItemTag::PHYSICAL_MAXIMUM); opt) item.physical[1] = *opt;
			if (auto opt = scopeValues.get(HIDReportGlobalItemTag::REPORT_SIZE); opt) item.size = *opt;
			if (auto opt = scopeValues.get(HIDReportGlobalItemTag::REPORT_COUNT); opt) item.count = *opt;

			break;
		}
		//case HIDReportMainItemTag::OUTPUT:
		//case HIDReportMainItemTag::FEATURE:
		default:
			break;
		}

		scopeValues.clearLocal();
	}

	void Gamepad::_parseRawReportItem(aurora::extensions::HIDReportScopeValues& scopeValues, aurora::extensions::HIDReportGlobalItemTag tag, int32_t val) {
		scopeValues.set(tag, val);
	}

	void Gamepad::_parseRawReportItem(aurora::extensions::HIDReportScopeValues& scopeValues, aurora::extensions::HIDReportLocalItemTag tag, int32_t val) {
		scopeValues.set(tag, val);
	}
	*/
}
#endif