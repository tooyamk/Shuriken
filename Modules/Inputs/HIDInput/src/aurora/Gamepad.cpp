#include "Gamepad.h"
#include "aurora/HID.h"
#include "aurora/Debug.h"

namespace aurora::modules::inputs::hid_input {
	Gamepad::Gamepad(Input& input, const DeviceInfo& info, extensions::HIDDevice& hid) : GamepadBase(input, info, hid) {
		auto raw = aurora::extensions::HID::getRawReportDescriptor(hid);
		_parseRawReportDescriptor(raw);
		int a = 1;
	}

	DeviceState::CountType Gamepad::getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const {
		switch (type) {
		case DeviceStateType::DEAD_ZONE:
		{
			if (values && count) {
				((DeviceStateValue*)values)[0] = _getDeadZone((GamepadKeyCode)code);

				return 1;
			}

			return 0;
		}
		default:
			return 0;
		}
	}

	DeviceState::CountType Gamepad::setState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) {
		switch (type) {
		case DeviceStateType::DEAD_ZONE:
		{
			if (values && count) {
				_setDeadZone((GamepadKeyCode)code, ((DeviceStateValue*)values)[0]);
				return 1;
			}

			return 0;
		}
		default:
			return 0;
		}
	}

	void Gamepad::_doInput(bool dispatchEvent, InputBuffer& inputBuffer, size_t inputBufferSize) {
		auto buf = inputBuffer;

		if (inputBufferSize > sizeof(InputStateBuffer)) inputBufferSize = sizeof(InputStateBuffer);

		if (!dispatchEvent) {
			std::scoped_lock lock(_inputStateMutex);

			memcpy(_inputState, buf, inputBufferSize);

			return;
		}

		auto val = buf[1] << 8 | buf[0];
		printdln(val);

		/*
		auto first = state[0];
		auto buf = state + 1;

		auto a9 = buf[9];
		auto a10 = buf[10];
		auto a11 = buf[11];

		auto btnSrcVal = buf[10];
		auto square = (btnSrcVal & 0b10000) ? 0xFF : 0;
		auto tri = (btnSrcVal & 0b10000000) ? 0xFF : 0;
		if (square != 0) {
			//printdln(btnVal);
		}
		*/
		//if (btnSrcVal != 128) printdln(btnSrcVal);
		//printdln(String::toString(inputBuffer, 16));

		//028101810080FB7D0080 00 000000CCCC
		//0281FF7F0080F97C0080 01 000000CCCC a
		//028101810080F97C0080 02 000000CCCC b
		//048201810080FB7D0080 04 000000CCCC x
		//02810181FE7EF97C0080 08 000000CCCC y

		int a = 1;
	}

	bool Gamepad::_doOutput() {
		return false;
	}

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
}