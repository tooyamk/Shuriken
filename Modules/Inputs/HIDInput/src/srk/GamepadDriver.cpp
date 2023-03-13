#include "GamepadDriver.h"

#if SRK_OS != SRK_OS_WINDOWS
#include "Input.h"
#include "srk/Debug.h"
#include "srk/HID.h"

namespace srk::modules::inputs::hid_input {
	GamepadDriver::GamepadDriver(Input& input, extensions::HIDDevice& hid) : GamepadDriverBase(input, hid) {}

	GamepadDriver::~GamepadDriver() {}

	GamepadDriver* GamepadDriver::create(Input& input, extensions::HIDDevice& hid, int32_t index) {
		using namespace srk::enum_operators;
		using namespace srk::extensions;

		auto ba = HID::getReportDescriptor(hid);
		printaln(ba.getLength());

		auto curIndex = -1;
		auto usagePage = (uint16_t)HIDReportUsagePageType::UNDEFINED;
		size_t reportSize = 0, reportID = 0, reportCount = 0, logicalMinimum = 0, logicalMaximum = 0;
		size_t usageMinimum = 0, usageMaximum = 0;
		std::vector<uint16_t> usages;

		uint32_t collection = 0;
		uint32_t inputBits = 0;
		HIDReportDescriptorItem item;

		DeviceDesc desc;
		desc.inputReportID = 0;

		while (ba.getBytesAvailable()) {
			if (auto n = HIDReportDescriptorItem::parse(ba.getCurrentSource(), ba.getBytesAvailable(), item); n) {
				auto p = ba.getPosition();
				ba.setPosition(p + n);

				switch (item.type) {
				case HIDReportItemType::MAIN:
				{
					switch ((HIDReportMainItemTag)item.tag) {
					case HIDReportMainItemTag::INPUT:
					{
						if (curIndex == index) {
							if (desc.inputReportID != reportID) {
								desc.inputReportID = reportID;
								inputBits += 8;
							}

							auto val = ba.read<ba_vt::UIX>(item.size);
							if ((val & 0b1) == 0) {
								switch ((HIDReportUsagePageType)usagePage) {
								case HIDReportUsagePageType::GENERIC_DESKTOP:
								{
									for (size_t i = 0, n = usages.size(); i < n; ++i) {
										switch ((HIDReportGenericDesktopPageType)usages[i]) {
										case HIDReportGenericDesktopPageType::X:
										case HIDReportGenericDesktopPageType::Y:
										case HIDReportGenericDesktopPageType::Z:
										case HIDReportGenericDesktopPageType::RX:
										case HIDReportGenericDesktopPageType::RY:
										case HIDReportGenericDesktopPageType::RZ:
										{
											auto& cap = desc.inputAxes.emplace_back();

											cap.offset = inputBits + i * reportSize;
											cap.size = reportSize;
											cap.min = logicalMinimum;
											cap.max = logicalMaximum;

											break;
										}
										case HIDReportGenericDesktopPageType::HAT_SWITCH:
										{
											auto& cap = desc.inputDPads.emplace_back();

											cap.offset = inputBits + i * reportSize;
											cap.size = reportSize;
											cap.min = logicalMinimum;
											cap.max = logicalMaximum;

											break;
										}
										default:
											break;
										}
									}

									break;
								}
								case HIDReportUsagePageType::BUTTON:
								{
									for (auto i = usageMinimum; i <= usageMaximum; ++i) {
										auto& cap = desc.inputButtons.emplace_back();

										cap.offset = inputBits + i * reportSize;
										cap.size = reportSize;
										cap.min = logicalMinimum;
										cap.max = logicalMaximum;
									}

									break;
								}
								default:
									break;
								}
							}
						}

						inputBits += reportCount * reportSize;

						break;
					}
					case HIDReportMainItemTag::OUTPUT:
						break;
					case HIDReportMainItemTag::COLLECTION:
						++collection;
						break;
					case HIDReportMainItemTag::END_COLLECTION:
						--collection;
						break;
					default:
						break;
					}

					usages.clear();
					usageMinimum = 0;
					usageMaximum = 0;

					break;
				}
				case HIDReportItemType::GLOBAL:
				{
					switch ((HIDReportGlobalItemTag)item.tag) {
					case HIDReportGlobalItemTag::USAGE_PAGE:
						usagePage = ba.read<ba_vt::UIX>(item.size);
						break;
					case HIDReportGlobalItemTag::LOGICAL_MINIMUM:
						logicalMinimum = ba.read<ba_vt::UIX>(item.size);
						break;
					case HIDReportGlobalItemTag::LOGICAL_MAXIMUM:
						logicalMaximum = ba.read<ba_vt::UIX>(item.size);
						break;
					case HIDReportGlobalItemTag::REPORT_SIZE:
						reportSize = ba.read<ba_vt::UIX>(item.size);
						break;
					case HIDReportGlobalItemTag::REPORT_ID:
						reportID = ba.read<ba_vt::UIX>(item.size);
						break;
					case HIDReportGlobalItemTag::REPORT_COUNT:
						reportCount = ba.read<ba_vt::UIX>(item.size);
						break;
					default:
						break;
					}

					break;
				}
				case HIDReportItemType::LOCAL:
				{
					switch ((HIDReportLocalItemTag)item.tag) {
					case HIDReportLocalItemTag::USAGE:
					{
						if (collection == 0) {
							++curIndex;
						} else {
							usages.emplace_back(ba.read<ba_vt::UIX>(item.size));
						}

						break;
					}
					case HIDReportLocalItemTag::USAGE_MINIMUM:
						usageMinimum = ba.read<ba_vt::UIX>(item.size);
						break;
					case HIDReportLocalItemTag::USAGE_MAXIMUM:
						usageMaximum = ba.read<ba_vt::UIX>(item.size);
						break;
					default:
						break;
					}

					break;
				}
				default:
					break;
				}

				ba.setPosition(p + n + item.size);
			} else {
				printaln("err ==============");
				break;
			}
		}

		if (curIndex < index) return nullptr;

		desc.inputReportLength = (inputBits + 7) >> 3;

		_toString(hid);

		return new GamepadDriver(input, hid);
	}

	size_t GamepadDriver::getInputLength() const {
		return 1;
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
		using namespace srk::extensions;

		uint8_t buf[10];
		if (HID::isSuccess(HID::read(*_hid, buf, 10, 0))) {
			printaln(111);
		}

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

	void GamepadDriver::_toString(extensions::HIDDevice& hid) {
		using namespace srk::enum_operators;
		using namespace srk::extensions;

		std::string RED_BEGIN = "\33[31m";
		std::string GREEN_BEGIN = "\33[32m";
		std::string COL_END = "\33[0m";
		std::string INDENT = "    ";

		auto ba = HID::getReportDescriptor(hid);
		printaln(ba.getLength());

		std::string info;
		std::string indent;

		auto lastUsagePage = HIDReportUsagePageType::UNDEFINED;

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
			auto end = info.size();
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
				indent += INDENT;
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
				indent.erase(indent.end() - INDENT.size(), indent.end());
				info.erase(info.begin() + end - INDENT.size(), info.begin() + end);
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
				lastUsagePage = (HIDReportUsagePageType)ba.read<ba_vt::UIX>(item.size);
				if (item.size) {
					switch (lastUsagePage) {
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
						if (lastUsagePage >= HIDReportUsagePageType::VENDOR_DEFINED_BEGIN && lastUsagePage <= HIDReportUsagePageType::VENDOR_DEFINED_END) {
							info += "vendor_defined(";
							info += String::toString((size_t)lastUsagePage);
							info += ")";
						} else {
							info += RED_BEGIN;
							info += "unknown(";
							info += String::toString((size_t)lastUsagePage);
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
				
				auto val = ba.read<ba_vt::UIX>(item.size);
				if (lastUsagePage == HIDReportUsagePageType::GENERIC_DESKTOP) {
					switch ((HIDReportGenericDesktopPageType)val) {
					case HIDReportGenericDesktopPageType::UNDEFINED:
						info += "undefined";
						break;
					case HIDReportGenericDesktopPageType::POINTER:
						info += "pointer";
						break;
					case HIDReportGenericDesktopPageType::MOUSE:
						info += "mouse";
						break;
					case HIDReportGenericDesktopPageType::JOYSTICK:
						info += "joystick";
						break;
					case HIDReportGenericDesktopPageType::GAMEPAD:
						info += "gamepad";
						break;
					case HIDReportGenericDesktopPageType::KEYBOARD:
						info += "keyboard";
						break;
					case HIDReportGenericDesktopPageType::KEYPAD:
						info += "keypad";
						break;
					case HIDReportGenericDesktopPageType::MULTI_AXIS_CONTROLLER:
						info += "multi_axis_controller";
						break;
					case HIDReportGenericDesktopPageType::TABLET_PC_SYSTEM_CONTROLS:
						info += "tablet_pc_system_controls";
						break;
					case HIDReportGenericDesktopPageType::WATER_COOLING_DEVICE:
						info += "water_cooling_device";
						break;
					case HIDReportGenericDesktopPageType::COMPUTER_CHASSIS_DEVICE:
						info += "computer_chassis_device";
						break;
					case HIDReportGenericDesktopPageType::WIRELESS_RADIO_CONTROLS:
						info += "wireless_radio_controls";
						break;
					case HIDReportGenericDesktopPageType::PORTABLE_DEVICE_CONTROLS:
						info += "portable_device_controls";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_MULTI_AXIS_CONTROLLER:
						info += "system_multi_axis_controller";
						break;
					case HIDReportGenericDesktopPageType::SPATIAL_CONTROLLER:
						info += "spatial_controller";
						break;
					case HIDReportGenericDesktopPageType::ASSISTIVE_CONTROL:
						info += "assistive_control";
						break;
					case HIDReportGenericDesktopPageType::DEVICE_DOCK:
						info += "device_dock";
						break;
					case HIDReportGenericDesktopPageType::DOCKABLE_DEVICE:
						info += "dockable_device";
						break;
					case HIDReportGenericDesktopPageType::X:
						info += "x";
						break;
					case HIDReportGenericDesktopPageType::Y:
						info += "y";
						break;
					case HIDReportGenericDesktopPageType::Z:
						info += "z";
						break;
					case HIDReportGenericDesktopPageType::RX:
						info += "rx";
						break;
					case HIDReportGenericDesktopPageType::RY:
						info += "ry";
						break;
					case HIDReportGenericDesktopPageType::RZ:
						info += "rz";
						break;
					case HIDReportGenericDesktopPageType::SLIDER:
						info += "slider";
						break;
					case HIDReportGenericDesktopPageType::DIAL:
						info += "dial";
						break;
					case HIDReportGenericDesktopPageType::WHEEL:
						info += "wheel";
						break;
					case HIDReportGenericDesktopPageType::HAT_SWITCH:
						info += "hat_switch";
						break;
					case HIDReportGenericDesktopPageType::COUNTED_BUFFER:
						info += "counted_buffer";
						break;
					case HIDReportGenericDesktopPageType::BYTE_COUNT:
						info += "byte_count";
						break;
					case HIDReportGenericDesktopPageType::MOTION_WAKEUP:
						info += "motion_wakeup";
						break;
					case HIDReportGenericDesktopPageType::START:
						info += "start";
						break;
					case HIDReportGenericDesktopPageType::SELECT:
						info += "select";
						break;
					case HIDReportGenericDesktopPageType::VX:
						info += "vx";
						break;
					case HIDReportGenericDesktopPageType::VY:
						info += "vy";
						break;
					case HIDReportGenericDesktopPageType::VZ:
						info += "vz";
						break;
					case HIDReportGenericDesktopPageType::VBRX:
						info += "vbrx";
						break;
					case HIDReportGenericDesktopPageType::VBRY:
						info += "vbry";
						break;
					case HIDReportGenericDesktopPageType::VBRZ:
						info += "vbrz";
						break;
					case HIDReportGenericDesktopPageType::VNO:
						info += "vno";
						break;
					case HIDReportGenericDesktopPageType::FEATURE_NOTIFICATION:
						info += "feature_notification";
						break;
					case HIDReportGenericDesktopPageType::RESOLUTION_MULTIPLIER:
						info += "resolution_multiplier";
						break;
					case HIDReportGenericDesktopPageType::QX:
						info += "qx";
						break;
					case HIDReportGenericDesktopPageType::QY:
						info += "qy";
						break;
					case HIDReportGenericDesktopPageType::QZ:
						info += "qz";
						break;
					case HIDReportGenericDesktopPageType::QW:
						info += "qw";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_CONTROL:
						info += "system_control";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_POWER_DOWN:
						info += "system_power_down";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_SLEEP:
						info += "system_sleep";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_WAKEUP:
						info += "system_wakeup";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_CONTEXT_MENU:
						info += "system_context_menu";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_MAIN_MENU:
						info += "system_main_menu";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_APP_MENU:
						info += "system_app_menu";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_MENU_HELP:
						info += "system_menu_help";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_MENU_EXIT:
						info += "system_menu_exit";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_MENU_SELECT:
						info += "system_menu_select";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_MENU_RIGHT:
						info += "system_menu_right";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_MENU_LEFT:
						info += "system_menu_left";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_MENU_UP:
						info += "system_menu_up";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_MENU_DOWN:
						info += "system_menu_down";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_COLD_RESTART:
						info += "system_cold_restart";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_WARM_RESTART:
						info += "system_warm_restart";
						break;
					case HIDReportGenericDesktopPageType::DPAD_UP:
						info += "dpad_up";
						break;
					case HIDReportGenericDesktopPageType::DPAD_DOWN:
						info += "dpad_down";
						break;
					case HIDReportGenericDesktopPageType::DPAD_RIGHT:
						info += "dpad_right";
						break;
					case HIDReportGenericDesktopPageType::DPAD_LEFT:
						info += "dpad_left";
						break;
					case HIDReportGenericDesktopPageType::INDEX_TRIGGER:
						info += "index_trigger";
						break;
					case HIDReportGenericDesktopPageType::PALM_TRIGGER:
						info += "palm_trigger";
						break;
					case HIDReportGenericDesktopPageType::THUMBSTICK:
						info += "thumbstick";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_FUNCTION_SHIFT:
						info += "system_function_shift";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_FUNCTION_SHIFT_LOCK:
						info += "system_function_shift_lock";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_FUNCTION_SHIFT_LOCK_INDICATOR:
						info += "system_function_shift_lock_indicator";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_DISMISS_NOTIFICATION:
						info += "system_dismiss_notification";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_DO_NOT_DISTURB:
						info += "system_do_not_disturb";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_DOCK:
						info += "system_dock";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_UNDOCK:
						info += "system_undock";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_SETUP:
						info += "system_setup";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_BREAK:
						info += "system_break";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_DEBUGGER_BREAK:
						info += "system_debugger_break";
						break;
					case HIDReportGenericDesktopPageType::APPLICATION_BREAK:
						info += "application_break";
						break;
					case HIDReportGenericDesktopPageType::APPLICATION_DEBUGGER_BREAK:
						info += "application_debugger_break";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_SPEATER_MUTE:
						info += "system_speater_mute";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_HIBERNATE:
						info += "system_hibernate";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_DISPLAY_INVERT:
						info += "system_display_invert";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_DISPLAY_INTERNAL:
						info += "system_display_internal";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_DISPLAY_EXTERNAL:
						info += "system_display_external";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_DISPLAY_BOTH:
						info += "system_display_both";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_DISPLAY_DUAL:
						info += "system_display_dual";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_DISPLAY_TOGGLE_INT_OR_EXT_MODE:
						info += "system_display_toggle_int_or_ext_mode";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_DISPLAY_SWAP_PRIMARY_OR_SECONDARY:
						info += "system_display_swap_primary_or_secondary";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_DISPLAY_TOGGLE_LCD_AUTOSCALE:
						info += "system_display_toggle_lcd_autoscale";
						break;
					case HIDReportGenericDesktopPageType::SENSOR_ZONE:
						info += "sensor_zone";
						break;
					case HIDReportGenericDesktopPageType::RPM:
						info += "rpm";
						break;
					case HIDReportGenericDesktopPageType::COOLANT_LEVEL:
						info += "coolant_level";
						break;
					case HIDReportGenericDesktopPageType::COOLANT_CRITICAL_LEVEL:
						info += "collant_critical_level";
						break;
					case HIDReportGenericDesktopPageType::COOLANT_PUMP:
						info += "coolant_pump";
						break;
					case HIDReportGenericDesktopPageType::CHASSIS_ENCLOSURE:
						info += "chassis_enclosure";
						break;
					case HIDReportGenericDesktopPageType::WIRELESS_RADIO_BUTTON:
						info += "wireless_ratio_button";
						break;
					case HIDReportGenericDesktopPageType::WIRELESS_RADIO_LED:
						info += "wireless_ratio_led";
						break;
					case HIDReportGenericDesktopPageType::WIRELESS_RADIO_SLIDER_SWITCH:
						info += "wireless_ratio_slider_switch";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_DISPLAY_ROTATION_LOCK_BUTTON:
						info += "system_display_rotation_lock_button";
						break;
					case HIDReportGenericDesktopPageType::SYSTEM_DISPLAY_ROTATION_LOCK_SLIDER_SWITCH:
						info += "system_display_rotation_lock_slider_switch";
						break;
					case HIDReportGenericDesktopPageType::CONTROL_ENABLE:
						info += "control_enable";
						break;
					case HIDReportGenericDesktopPageType::DOCKABLE_DEVICE_UNIQUE_ID:
						info += "dockable_device_unique_id";
						break;
					case HIDReportGenericDesktopPageType::DOCKABLE_DEVICE_VENDOR_ID:
						info += "dockable_device_vendor_id";
						break;
					case HIDReportGenericDesktopPageType::DOCKABLE_DEVICE_PRIMARY_USAGE_PAGE:
						info += "dockable_device_primary_usage_page";
						break;
					case HIDReportGenericDesktopPageType::DOCKABLE_DEVICE_PRIMARY_USAGE_ID:
						info += "dockable_device_primary_usage_id";
						break;
					case HIDReportGenericDesktopPageType::DOCKABLE_DEVICE_DOCKING_STATE:
						info += "dockable_device_docking_state";
						break;
					case HIDReportGenericDesktopPageType::DOCKABLE_DEVICE_DISPLAY_OCCLUSION:
						info += "dockable_device_display_occlusion";
						break;
					case HIDReportGenericDesktopPageType::DOCKABLE_DEVICE_OBJECT_TYPE:
						info += "dockable_device_object_type";
						break;
					default:
					{
						info += RED_BEGIN;
						info += "unknown(";
						info += String::toString(val);
						info += ")";
						info += COL_END;

						break;
					}
					}
				} else {
					info += RED_BEGIN;
					info += "unknown(";
					info += String::toString(val);
					info += ")";
					info += COL_END;
				}

				break;
			}
			case HIDReportLocalItemTag::USAGE_MINIMUM:
			{
				info += "usage_minimum";

				info += "  ";
				info += String::toString(ba.read<ba_vt::UIX>(item.size));

				break;
			}
			case HIDReportLocalItemTag::USAGE_MAXIMUM:
			{
				info += "usage_maximum";

				info += "  ";
				info += String::toString(ba.read<ba_vt::UIX>(item.size));

				break;
			}
			case HIDReportLocalItemTag::DESIGNATOR_INDEX:
			{
				info += "designator_index";

				info += "  ";
				info += String::toString(ba.read<ba_vt::UIX>(item.size));

				break;
			}
			case HIDReportLocalItemTag::DESIGNATOR_MINIMUM:
			{
				info += "designator_minimum";

				info += "  ";
				info += String::toString(ba.read<ba_vt::UIX>(item.size));

				break;
			}
			case HIDReportLocalItemTag::DESIGNATOR_MAXIMUM:
			{
				info += "designator_maximum";

				info += "  ";
				info += String::toString(ba.read<ba_vt::UIX>(item.size));

				break;
			}
			case HIDReportLocalItemTag::STRING_INDEX:
			{
				info += "string_index";

				info += "  ";
				info += String::toString(ba.read<ba_vt::UIX>(item.size));

				break;
			}
			case HIDReportLocalItemTag::STRING_MINIMUM:
			{
				info += "string_minimum";

				info += "  ";
				info += String::toString(ba.read<ba_vt::UIX>(item.size));

				break;
			}
			case HIDReportLocalItemTag::STRING_MAXIMUM:
			{
				info += "string_maximum";

				info += "  ";
				info += String::toString(ba.read<ba_vt::UIX>(item.size));

				break;
			}
			case HIDReportLocalItemTag::DELIMITER:
			{
				info += "delimiter";

				info += "  ";
				switch (auto val = ba.read<ba_vt::UIX>(item.size)) {
				case 0:
					info += "close_set(0)";
					break;
				case 1:
					info += "open_set(1)";
					break;
				default:
					{
						info += GREEN_BEGIN;
						info += "unknown_set(";
						info += String::toString(val);
						info += ")";
						info += COL_END;

						break;
					}
				}

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
			info += indent;

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
	}
}
#endif