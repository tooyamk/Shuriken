#include "GamepadDriver.h"

#if SRK_OS != SRK_OS_WINDOWS
#include "Input.h"
#include "srk/Debug.h"
#include "srk/HID.h"

namespace srk::modules::inputs::hid_input {
	GamepadDriver::GamepadDriver(Input& input, extensions::HIDDevice& hid) : GamepadDriverBase(input, hid) {}

	GamepadDriver::~GamepadDriver() {}

	GamepadDriver* GamepadDriver::create(Input& input, extensions::HIDDevice& hid) {
		using namespace srk::enum_operators;
		using namespace srk::extensions;

		std::string RED_BEGIN = "\33[31m";
		std::string GREEN_BEGIN = "\33[32m";
		std::string COL_END = "\33[0m";

		auto ba = HID::getRawReportDescriptor(hid);
		printaln(ba.getLength());

		std::string info;

		auto inputOutputFeatureOpt = [&](uint32_t val) {
			info += "  ";
			info += val & 0b1 ? "Constant(1)" : "Data(0)";
			info += "|";
			info += val & 0b10 ? "Variable(1)" : "Array(0)";
			info += "|";
			info += val & 0b100 ? "Relative(1)" : "Absolute(0)";
			info += "|";
			info += val & 0b1000 ? "Wrap(1)" : "No Wrap(0)";
			info += "|";
			info += val & 0b10000 ? "Non Linear(1)" : "Linear(0)";
			info += "|";
			info += val & 0b100000 ? "No Preferred(1)" : "Preferred State(0)";
			info += "|";
			info += val & 0b1000000 ? "Nullstate(1)" : "No Null position(0)";
			info += "|";
			info += val & 0b100000000 ? "Buffered Bytes(1)" : "Bit Field(0)";
		};

		auto itemMainFn = [&](const HIDReportDescriptorItem& item) {
			info += "main     ";

			switch ((HIDReportMainItemTag)item.tag) {
			case HIDReportMainItemTag::INPUT:
			{
				info += "input";
				inputOutputFeatureOpt(ba.read<ba_vt::UIX>(item.size));

				break;
			}
			case HIDReportMainItemTag::OUTPUT:
			{
				info += "output";
				inputOutputFeatureOpt(ba.read<ba_vt::UIX>(item.size));

				break;
			}
			case HIDReportMainItemTag::COLLECTION:
			{
				info += "collection  ";

				if (item.size) {
					if (item.size == 1) {
						auto cd = (HIDReportCollectionData)ba.read<uint8_t>();
						switch (cd) {
						case HIDReportCollectionData::PHYSICAL:
							info += "physical";
							break;
						case HIDReportCollectionData::APPLICATION:
							info += "application";
							break;
						case HIDReportCollectionData::LOGICAL:
							info += "logical";
							break;
						case HIDReportCollectionData::REPORT:
							info += "report";
							break;
						case HIDReportCollectionData::NAMED_ARRAY:
							info += "named_array";
							break;
						case HIDReportCollectionData::USAGE_MODIFIER:
							info += "usage_modifier";
							break;
						case HIDReportCollectionData::USAGE_SWITCH:
							info += "usage_switch";
							break;
						default:
						{
							info += RED_BEGIN;

							if (cd >= HIDReportCollectionData::RESERVED_BEGIN && cd <= HIDReportCollectionData::RESERVED_END) {
								info += "reserved(";
								info += String::toString((size_t)cd);
								info += ")";
							} else {
								info += "vendor_defined(";
								info += String::toString((size_t)cd);
								info += ")";
							}

							info += COL_END;

							break;
						}
						}
					} else {
						info += RED_BEGIN;
						info += "unknown";
						info += COL_END;
					}
				}

				break;
			}
			case HIDReportMainItemTag::FEATURE:
			{
				info += "feature";
				inputOutputFeatureOpt(ba.read<ba_vt::UIX>(item.size));

				break;
			}
			case HIDReportMainItemTag::END_COLLECTION:
			{
				info += "end_collection";

				info += "  ";
				info += "size : ";
				info += String::toString(item.size);

				break;
			}
			default:
			{
				info += RED_BEGIN;
				info += "unknown(";
				info += String::toString(item.tag);
				info += ")";

				info += "  ";
				info += "size : ";
				info += String::toString(item.size);
				info += COL_END;

				break;
			}
			}

			info += "\n";
		};

		auto itemGlobalFn = [&](const HIDReportDescriptorItem& item) {
			info += "global   ";

			switch ((HIDReportGlobalItemTag)item.tag) {
			case HIDReportGlobalItemTag::USAGE_PAGE:
			{
				info += "usage_page  ";

				if (item.size) {
					switch (auto up = (HIDReportUsagePageType)ba.read<ba_vt::UIX>(item.size)) {
					case HIDReportUsagePageType::UNDEFINED:
						info += "undefined";
						break;
					case HIDReportUsagePageType::GENERIC_DESKTOP:
						info += "generic_desktop";
						break;
					case HIDReportUsagePageType::SIMULATION_CONTROLS:
						info += "simulation_contrils";
						break;
					case HIDReportUsagePageType::VR_CONTROLS:
						info += "vr_controls";
						break;
					case HIDReportUsagePageType::SPORT_CONTROLS:
						info += "sport_controls";
						break;
					case HIDReportUsagePageType::GAME_CONTROLS:
						info += "game_controls";
						break;
					case HIDReportUsagePageType::GENERIC_DEVICE_CONTROLS:
						info += "generic_device_controls";
						break;
					case HIDReportUsagePageType::KEYBOARD_OR_KEYPAD:
						info += "keyboard_or_keypad";
						break;
					case HIDReportUsagePageType::LEDS:
						info += "leds";
						break;
					case HIDReportUsagePageType::BUTTON:
						info += "button";
						break;
					case HIDReportUsagePageType::ORDINALS:
						info += "ordinals";
						break;
					case HIDReportUsagePageType::TELEPHONY_DEVICES:
						info += "telephony_devices";
						break;
					case HIDReportUsagePageType::CONSUMER_DEVICES:
						info += "consumer_devices";
						break;
					case HIDReportUsagePageType::DIGITIZERS:
						info += "digitizers";
						break;
					case HIDReportUsagePageType::HAPTICS:
						info += "haptics";
						break;
					case HIDReportUsagePageType::PID:
						info += "pid";
						break;
					case HIDReportUsagePageType::UNICODE:
						info += "unicode";
						break;
					case HIDReportUsagePageType::EYE_AND_HEAD_TRACKERS:
						info += "eye_and_head_trackers";
						break;
					case HIDReportUsagePageType::AUXILIARY_DISPLAY:
						info += "auxiliary_display";
						break;
					case HIDReportUsagePageType::SENSORS:
						info += "sensors";
						break;
					case HIDReportUsagePageType::MEDIA_INSTRUMENT:
						info += "media_instrument";
						break;
					case HIDReportUsagePageType::BRAILLE_DISPLAY:
						info += "braille_display";
						break;
					case HIDReportUsagePageType::LIGHTING_AND_ILLUMINATION:
						info += "lighting_and_illumination";
						break;
					case HIDReportUsagePageType::MONITOR:
						info += "monitor";
						break;
					case HIDReportUsagePageType::MONITOR_ENUMERATED_VALUES:
						info += "monitor_enumerated_values";
						break;
					case HIDReportUsagePageType::VESA_VIRTUAL_CONTROLS:
						info += "vesa_virtual_controls";
						break;
					case HIDReportUsagePageType::VESA_COMMAND:
						info += "vesa_command";
						break;
					case HIDReportUsagePageType::POWER_DEVICE:
						info += "power_device";
						break;
					case HIDReportUsagePageType::BATTERY_SYSTEM:
						info += "battery_system";
						break;
					case HIDReportUsagePageType::POWER_PAGES_BEGIN:
						info += "power_pages_begin";
						break;
					case HIDReportUsagePageType::POWER_PAGES_END:
						info += "power_pages_end";
						break;
					case HIDReportUsagePageType::BAR_CODE_SCANNER:
						info += "bar_code_scanner";
						break;
					case HIDReportUsagePageType::SCALE:
						info += "scale";
						break;
					case HIDReportUsagePageType::MAGNETIC_STRIPE_REDING_DEVICES:
						info += "magnetic_stripe_reding_devices";
						break;
					case HIDReportUsagePageType::RESERVED_POINT_OF_SALE:
						info += "reserved_point_of_sale";
						break;
					case HIDReportUsagePageType::CAMERA_CONTROL:
						info += "camera_control";
						break;
					case HIDReportUsagePageType::ARCADE:
						info += "arcade";
						break;
					case HIDReportUsagePageType::GAMING_DEVICE:
						info += "gaming_device";
						break;
					case HIDReportUsagePageType::FIDO_ALLIANCE:
						info += "pido_alliance";
						break;
					default:
					{
						if (up >= HIDReportUsagePageType::VENDOR_DEFINED_BEGIN && up <= HIDReportUsagePageType::VENDOR_DEFINED_END) {
							info += "vendor_defined(";
							info += String::toString((size_t)up);
							info += ")";
						} else {
							info += RED_BEGIN;
							info += "unknown(";
							info += String::toString((size_t)up);
							info += ")";
							info += COL_END;
						}
					}
					}
				}

				break;
			}
			case HIDReportGlobalItemTag::LOGICAL_MINIMUM:
			{
				info += "logical_minimum";

				info += "  ";
				info += String::toString(ba.read<ba_vt::UIX>(item.size));

				break;
			}
			case HIDReportGlobalItemTag::LOGICAL_MAXIMUM:
			{
				info += "logical_maximum";

				info += "  ";
				info += String::toString(ba.read<ba_vt::UIX>(item.size));

				break;
			}
			case HIDReportGlobalItemTag::PHYSICAL_MINIMUM:
			{
				info += "physical_miunimum";

				info += "  ";
				info += String::toString(ba.read<ba_vt::UIX>(item.size));

				break;
			}
			case HIDReportGlobalItemTag::PHYSICAL_MAXIMUM:
			{
				info += "physical_maximum";

				info += "  ";
				info += String::toString(ba.read<ba_vt::UIX>(item.size));

				break;
			}
			case HIDReportGlobalItemTag::UNIT_EXPONENT:
			{
				info += "unit_exponent";

				info += "  ";
				info += String::toString(ba.read<ba_vt::UIX>(item.size));

				break;
			}
			case HIDReportGlobalItemTag::UNIT:
			{
				info += "unit";

				info += "  ";
				info += String::toString(ba.read<ba_vt::UIX>(item.size));

				break;
			}
			case HIDReportGlobalItemTag::REPORT_SIZE:
			{
				info += "report_size";

				info += "  ";
				info += String::toString(ba.read<ba_vt::UIX>(item.size));

				break;
			}
			case HIDReportGlobalItemTag::REPORT_ID:
			{
				info += "report_id";

				info += "  ";
				info += String::toString(ba.read<ba_vt::UIX>(item.size));

				break;
			}
			case HIDReportGlobalItemTag::REPORT_COUNT:
			{
				info += "report_count";

				info += "  ";
				info += String::toString(ba.read<ba_vt::UIX>(item.size));

				break;
			}
			case HIDReportGlobalItemTag::PUSH:
			{
				info += "push";

				break;
			}
			case HIDReportGlobalItemTag::POP:
			{
				info += "pop";

				break;
			}
			default:
			{
				info += RED_BEGIN;

				info += "reserved(";
				info += String::toString(item.tag);
				info += ")";

				info += "size : ";
				info += String::toString(item.size);

				info += COL_END;

				break;
			}
			}

			info += "\n";
		};

		auto itemLocalFn = [&](const HIDReportDescriptorItem& item) {
			info += "local    ";

			switch ((HIDReportLocalItemTag)item.tag) {
			case HIDReportLocalItemTag::USAGE:
			{
				info += "usage";

				info += "  ";
				info += GREEN_BEGIN;
				info += "todo";
				info += COL_END;

				break;
			}
			case HIDReportLocalItemTag::USAGE_MINIMUM:
			{
				info += "usage_minimum";

				info += "  ";
				info += GREEN_BEGIN;
				info += "todo";
				info += COL_END;

				break;
			}
			case HIDReportLocalItemTag::USAGE_MAXIMUM:
			{
				info += "usage_maximum";

				info += "  ";
				info += GREEN_BEGIN;
				info += "todo";
				info += COL_END;

				break;
			}
			case HIDReportLocalItemTag::DESIGNATOR_INDEX:
			{
				info += "designator_index";

				info += "  ";
				info += GREEN_BEGIN;
				info += "todo";
				info += COL_END;

				break;
			}
			case HIDReportLocalItemTag::DESIGNATOR_MINIMUM:
			{
				info += "designator_minimum";

				info += "  ";
				info += GREEN_BEGIN;
				info += "todo";
				info += COL_END;

				break;
			}
			case HIDReportLocalItemTag::DESIGNATOR_MAXIMUM:
			{
				info += "designator_maximum";

				info += "  ";
				info += GREEN_BEGIN;
				info += "todo";
				info += COL_END;

				break;
			}
			case HIDReportLocalItemTag::STRING_INDEX:
			{
				info += "string_index";

				info += "  ";
				info += GREEN_BEGIN;
				info += "todo";
				info += COL_END;

				break;
			}
			case HIDReportLocalItemTag::STRING_MINIMUM:
			{
				info += "string_minimum";

				info += "  ";
				info += GREEN_BEGIN;
				info += "todo";
				info += COL_END;

				break;
			}
			case HIDReportLocalItemTag::STRING_MAXIMUM:
			{
				info += "string_maximum";

				info += "  ";
				info += GREEN_BEGIN;
				info += "todo";
				info += COL_END;

				break;
			}
			case HIDReportLocalItemTag::DELIMITER:
			{
				info += "delimiter";

				info += "  ";
				info += GREEN_BEGIN;
				info += "todo";
				info += COL_END;

				break;
			}
			default:
			{
				info += RED_BEGIN;
				info += "reserved(";
				info += String::toString(item.tag);
				info += ")";
				info += COL_END;

				break;
			}
			}

			info += "\n";
		};

		auto itemReservedFn = [&](const HIDReportDescriptorItem& item) {
			info += RED_BEGIN;
			info += "reserved ";

			info += "tag : ";
			info += String::toString(item.tag);
			info += "  ";

			info += "size : ";
			info += String::toString(item.size);

			info += COL_END;
			info += "\n";
		};

		auto itemUnknownFn = [&](const HIDReportDescriptorItem& item) {
			info += RED_BEGIN;
			info += "unknown(";
			info += String::toString((size_t)item.type);
			info += ")  ";

			info += "tag : ";
			info += String::toString(item.tag);
			info += "  ";

			info += "size : ";
			info += String::toString(item.size);

			info += COL_END;
			info += "\n";
		};

		auto itemFn = [&](const HIDReportDescriptorItem& item) {
			info += "item : ";
			switch (item.type) {
			case HIDReportItemType::MAIN:
				itemMainFn(item);
				break;
			case HIDReportItemType::GLOBAL:
				itemGlobalFn(item);
				break;
			case HIDReportItemType::LOCAL:
				itemLocalFn(item);
				break;
			case HIDReportItemType::RESERVED:
				itemReservedFn(item);
				break;
			default:
				itemUnknownFn(item);
				break;
			}
		};

		HIDReportDescriptorItem item;
		while (ba.getBytesAvailable()) {
			if (auto n = HIDReportDescriptorItem::parse(ba.getCurrentSource(), ba.getBytesAvailable(), item); n) {
				auto p = ba.getPosition();
				ba.setPosition(p + n);
				itemFn(item);

				ba.setPosition(p + n + item.size);
			} else {
				printaln("err ==============");
				break;
			}
		}

		printaln(info);

		return nullptr;
	}

	size_t GamepadDriver::getInputLength() const {
		return 0;
	}

	size_t GamepadDriver::getOutputLength() const {
		return 0;
	}

	bool GamepadDriver::init(void* inputState, void* outputState) {
		return false;
	}

	bool GamepadDriver::isStateReady(const void* state) const {
		return false;
	}

	bool GamepadDriver::readStateFromDevice(void* inputState) const {
		return false;
	}

	float32_t GamepadDriver::readDataFromInputState(const void* inputState, GamepadKeyCodeAndFlags cf, float32_t defaultVal) const {
		return translate(defaultVal, cf.flags);
	}

	DeviceState::CountType GamepadDriver::customGetState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count,
		const void* inputState, void* custom, ReadWriteStateStartCallback readStateStartCallback, ReadWriteStateStartCallback readStateEndCallback) const {
		return 0;
	}

	void GamepadDriver::customDispatch(const void* oldInputState, const void* newInputState, void* custom, DispatchCallback dispatchCallback) const {}

	bool GamepadDriver::writeStateToDevice(const void* outputState) const {
		return false;
	}

	DeviceState::CountType GamepadDriver::customSetState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count, void* outputState, void* custom,
		ReadWriteStateStartCallback writeStateStartCallback, ReadWriteStateStartCallback writeStateEndCallback) const {
		return 0;
	}

	void GamepadDriver::setKeyMapper(GamepadKeyMapper& dst, const GamepadKeyMapper* src) const {
		if (src) {
			dst = *src;
		} else {
			dst.clear();
		}
	}
}
#endif