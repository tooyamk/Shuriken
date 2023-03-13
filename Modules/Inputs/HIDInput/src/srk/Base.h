#pragma once

#include "srk/modules/inputs/InputModule.h"
#include "srk/HID.h"
#include <shared_mutex>

namespace srk::modules::inputs::hid_input {
	class SRK_MODULE_DLL InternalDeviceInfo : public DeviceInfo {
	public:
		std::string path;
		int32_t index;
	};
}

/*
#ifndef SRK_GENERIC_INPUT_ENUM
#	define SRK_GENERIC_INPUT_ENUM

#	define SRK_GENERIC_INPUT_ENUM_HIDReportItemType \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(MAIN, 0) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(GLOBAL, 1) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(LOCAL, 2) \

#	define SRK_GENERIC_INPUT_ENUM_HIDReportMainItemTag \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(INPUT, 0x8) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(OUTPUT, 0x9) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(COLLECTION, 0xA) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(FEATURE, 0xB) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(END_COLLECTION, 0xC) \

#	define SRK_GENERIC_INPUT_ENUM_HIDReportGlobalItemTag \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(USAGE_PAGE, 0x0) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(LOGICAL_MINIMUM, 0x1) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(LOGICAL_MAXIMUM, 0x2) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(PHYSICAL_MINIMUM, 0x3) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(PHYSICAL_MAXIMUM, 0x4) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(UNIT_EXPONENT, 0x5) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(UNIT, 0x6) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(REPORT_SIZE, 0x7) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(REPORT_ID, 0x8) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(REPORT_COUNT, 0x9) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(PUSH, 0xA) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(POP, 0xB) \

#	define SRK_GENERIC_INPUT_ENUM_HIDReportUsagePageType \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(UNDEFINED, 0x0) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(GENERIC_DESKTOP, 0x1) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SIMULATION_CONTROLS, 0x2) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(VR_CONTROLS, 0x3) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SPORT_CONTROLS, 0x4) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(GAME_CONTROLS, 0x5) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(GENERIC_DEVICE_CONTROLS, 0x6) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(KEYBOARD_OR_KEYPAD, 0x7) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(LEDS, 0x8) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(BUTTON, 0x9) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(ORDINALS, 0xA) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(TELEPHONY_DEVICES, 0xB) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(CONSUMER_DEVICES, 0xC) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(DIGITIZERS, 0xD) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(HAPTICS, 0xE) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(PID, 0xF) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(UNICODE, 0x10) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(EYE_AND_HEAD_TRACKERS, 0x12) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(AUXILIARY_DISPLAY, 0x14) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SENSORS, 0x20) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(MEDIA_INSTRUMENT, 0x40) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(BRAILLE_DISPLAY, 0x41) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(LIGHTING_AND_ILLUMINATION, 0x59) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(MONITOR, 0x80) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(MONITOR_ENUMERATED_VALUES, 0x81) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(VESA_VIRTUAL_CONTROLS, 0x82) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(VESA_COMMAND, 0x83) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(POWER_DEVICE, 0x84) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(BATTERY_SYSTEM, 0x85) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(POWER_PAGES_BEGIN, 0x86) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(POWER_PAGES_END, 0x87) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(BAR_CODE_SCANNER, 0x8C) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SCALE, 0x8D) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(MAGNETIC_STRIPE_REDING_DEVICES, 0x8E) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(RESERVED_POINT_OF_SALE, 0x8F) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(CAMERA_CONTROL, 0x90) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(ARCADE, 0x91) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(GAMING_DEVICE, 0x92) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(FIDO_ALLIANCE, 0xF1D0) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(VENDOR_DEFINED_BEGIN, 0xFF00) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(VENDOR_DEFINED_END, 0xFFFF) \

#	define SRK_GENERIC_INPUT_ENUM_HIDReportGenericDesktopPageType \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(UNDEFINED, 0x0) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(POINTER, 0x1) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(MOUSE, 0x2) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(JOYSTICK, 0x4) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(GAMEPAD, 0x5) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(KEYBOARD, 0x6) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(KEYPAD, 0x7) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(MULTI_AXIS_CONTROLLER, 0x8) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(TABLET_PC_SYSTEM_CONTROLS, 0x9) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(WATER_COOLING_DEVICE, 0xA) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(COMPUTER_CHASSIS_DEVICE, 0xB) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(WIRELESS_RADIO_CONTROLS, 0xC) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(PORTABLE_DEVICE_CONTROLS, 0xD) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_MULTI_AXIS_CONTROLLER, 0xE) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SPATIAL_CONTROLLER, 0xF) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(ASSISTIVE_CONTROL, 0x10) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(DEVICE_DOCK, 0x11) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(DOCKABLE_DEVICE, 0x12) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(X, 0x30) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(Y, 0x31) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(Z, 0x32) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(R_X, 0x33) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(R_Y, 0x34) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(R_Z, 0x35) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SLIDER, 0x36) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(DIAL, 0x37) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(WHEEL, 0x38) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(HAT_SWITCH, 0x39) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(COUNTED_BUFFER, 0x3A) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(BYTE_COUNT, 0x3B) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(MOTION_WAKEUP, 0x3C) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(START, 0x3D) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SELECT, 0x3E) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(V_X, 0x40) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(V_Y, 0x41) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(V_Z, 0x42) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(V_BRX, 0x43) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(V_BRY, 0x44) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(V_BRZ, 0x45) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(V_NO, 0x46) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(FEATURE_NOTIFICATION, 0x47) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(RESOLUTION_MULTIPLIER, 0x48) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(Q_X, 0x49) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(Q_Y, 0x4A) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(Q_Z, 0x4B) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(Q_W, 0x4C) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_CONTROL, 0x80) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_POWER_DOWN, 0x81) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_SLEEP, 0x82) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_WAKEUP, 0x83) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_CONTEXT_MENU, 0x84) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_MAIN_MENU, 0x85) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_APP_MENU, 0x86) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_MENU_HELP, 0x87) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_MENU_EXIT, 0x88) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_MENU_SELECT, 0x89) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_MENU_RIGHT, 0x8A) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_MENU_LEFT, 0x8B) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_MENU_UP, 0x8C) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_MENU_DOWN, 0x8D) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_COLD_RESTART, 0x8E) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_WARM_RESTART, 0x8F) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(D_PAD_UP, 0x90) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(D_PAD_DOWN, 0x91) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(D_PAD_RIGHT, 0x92) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(D_PAD_LEFT, 0x93) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(INDEX_TRIGGER, 0x94) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(PALM_TRIGGER, 0x95) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(THUMBSTICK, 0x96) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_FUNCTION_SHIFT, 0x97) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_FUNCTION_SHIFT_LOCK, 0x98) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_FUNCTION_SHIFT_LOCK_INDICATOR, 0x99) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_DISMISS_NOTIFICATION, 0x9A) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_DO_NOT_DISTURB, 0x9B) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_DOCK, 0xA0) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_UNDOCK, 0xA1) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_SETUP, 0xA2) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_BREAK, 0xA3) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_DEBUGGER_BREAK, 0xA4) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(APPLICATION_BREAK, 0xA5) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(APPLICATION_DEBUGGER_BREAK, 0xA6) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_SPEATER_MUTE, 0xA7) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_HIBERNATE, 0xA8) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_DISPLAY_INVERT, 0xB0) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_DISPLAY_INTERNAL, 0xB1) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_DISPLAY_EXTERNAL, 0xB2) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_DISPLAY_BOTH, 0xB3) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_DISPLAY_DUAL, 0xB4) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_DISPLAY_TOGGLE_INT_OR_EXT_MODE, 0xB5) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_DISPLAY_SWAP_PRIMARY_OR_SECONDARY, 0xB6) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_DISPLAY_TOGGLE_LCD_AUTOSCALE, 0xB7) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SENSOR_ZONE, 0xC0) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(RPM, 0xC1) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(COOLANT_LEVEL, 0xC2) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(COOLANT_CRITICAL_LEVEL, 0xC3) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(COOLANT_PUMP, 0xC4) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(CHASSIS_ENCLOSURE, 0xC5) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(WIRELESS_RADIO_BUTTON, 0xC6) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(WIRELESS_RADIO_LED, 0xC7) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(WIRELESS_RADIO_SLIDER_SWITCH, 0xC8) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_DISPLAY_ROTATION_LOCK_BUTTON, 0xC9) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(SYSTEM_DISPLAY_ROTATION_LOCK_SLIDER_SWITCH, 0xCA) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(CONTROL_ENABLE, 0xCB) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(DOCKABLE_DEVICE_UNIQUE_ID, 0xD0) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(DOCKABLE_DEVICE_VENDOR_ID, 0xD1) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(DOCKABLE_DEVICE_PRIMARY_USAGE_PAGE, 0xD2) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(DOCKABLE_DEVICE_PRIMARY_USAGE_ID, 0xD3) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(DOCKABLE_DEVICE_DOCKING_STATE, 0xD4) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(DOCKABLE_DEVICE_DISPLAY_OCCLUSION, 0xD5) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(DOCKABLE_DEVICE_OBJECT_TYPE, 0xD6) \

#	define SRK_GENERIC_INPUT_ENUM_HIDReportConsumerPageType \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(UNDEFINED, 0x0) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(CONSUMER_CONTROL, 0x1) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(NUMERIC_KEY_PAD, 0x2) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(PROGRAMMABLE_BUTTONS, 0x3) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(MICROPHONE, 0x4) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(HEADPHONE, 0x5) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(GRAPHIC_EQUALIZER, 0x6) \

#	define SRK_GENERIC_INPUT_ENUM_HIDReportLocalItemTag \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(USAGE, 0x0) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(USAGE_MINIMUM, 0x1) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(USAGE_MAXIMUM, 0x2) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(DESIGNATOR_INDEX, 0x3) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(DESIGNATOR_MINIMUM, 0x4) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(DESIGNATOR_MAXIMUM, 0x5) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(STRING_INDEX, 0x6) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(STRING_MINIMUM, 0x7) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(STRING_MAXIMUM, 0x8) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(DELIMITER, 0xA) \

#	define SRK_GENERIC_INPUT_ENUM_HIDReportCollectionData \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(PHYSICAL, 0x0) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(APPLICATION, 0x1) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(LOGICAL, 0x2) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(REPORT, 0x3) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(NAMED_ARRAY, 0x4) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(USAGE_MODIFIER, 0x5) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(USAGE_SWITCH, 0x6) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(VENDOR_DEFINED_BEGIN, 0x80) \
			SRK_GENERIC_INPUT_ENUM_ELEMENT(VENDOR_DEFINED_END, 0xFF) \

#endif

namespace srk::modules::inputs::generic_input {
	using namespace std::literals;

	enum class HIDReportItemType : uint8_t {
#define SRK_GENERIC_INPUT_ENUM_ELEMENT(a, b) a = b,
		SRK_GENERIC_INPUT_ENUM_HIDReportItemType
#undef SRK_GENERIC_INPUT_ENUM_ELEMENT
	};

	inline static const std::unordered_map<HIDReportItemType, std::string_view> HID_REPORT_ITEM_TYPE_MAP = {
#define SRK_GENERIC_INPUT_ENUM_ELEMENT(a, b) { HIDReportItemType::a, #a##sv },
			SRK_GENERIC_INPUT_ENUM_HIDReportItemType
#undef SRK_GENERIC_INPUT_ENUM_ELEMENT
			{ (HIDReportItemType)(std::numeric_limits<std::underlying_type_t<HIDReportItemType>>::max)(), "__end"sv }
	};

	enum class HIDReportMainItemTag : uint8_t {
#define SRK_GENERIC_INPUT_ENUM_ELEMENT(a, b) a = b,
		SRK_GENERIC_INPUT_ENUM_HIDReportMainItemTag
#undef SRK_GENERIC_INPUT_ENUM_ELEMENT
	};

	inline static const std::unordered_map<HIDReportMainItemTag, std::string_view> HID_REPORT_MAIN_ITEM_TAG_MAP = {
#define SRK_GENERIC_INPUT_ENUM_ELEMENT(a, b) { HIDReportMainItemTag::a, #a##sv },
			SRK_GENERIC_INPUT_ENUM_HIDReportMainItemTag
#undef SRK_GENERIC_INPUT_ENUM_ELEMENT
			{ (HIDReportMainItemTag)(std::numeric_limits<std::underlying_type_t<HIDReportMainItemTag>>::max)(), "__end"sv }
	};

	enum class HIDReportGlobalItemTag : uint8_t {
#define SRK_GENERIC_INPUT_ENUM_ELEMENT(a, b) a = b,
		SRK_GENERIC_INPUT_ENUM_HIDReportGlobalItemTag
#undef SRK_GENERIC_INPUT_ENUM_ELEMENT
	};

	inline static const std::unordered_map<HIDReportGlobalItemTag, std::string_view> HID_REPORT_GLOBAL_ITEM_TAG_MAP = {
#define SRK_GENERIC_INPUT_ENUM_ELEMENT(a, b) { HIDReportGlobalItemTag::a, #a##sv },
			SRK_GENERIC_INPUT_ENUM_HIDReportGlobalItemTag
#undef SRK_GENERIC_INPUT_ENUM_ELEMENT
			{ (HIDReportGlobalItemTag)(std::numeric_limits<std::underlying_type_t<HIDReportGlobalItemTag>>::max)(), "__end"sv }
	};

	enum class HIDReportUsagePageType : uint32_t {
#define SRK_GENERIC_INPUT_ENUM_ELEMENT(a, b) a = b,
		SRK_GENERIC_INPUT_ENUM_HIDReportUsagePageType
#undef SRK_GENERIC_INPUT_ENUM_ELEMENT
	};

	inline static const std::unordered_map<HIDReportUsagePageType, std::string_view> HID_REPORT_USAGE_PAGE_TYPE_MAP = {
#define SRK_GENERIC_INPUT_ENUM_ELEMENT(a, b) { HIDReportUsagePageType::a, #a##sv },
			SRK_GENERIC_INPUT_ENUM_HIDReportUsagePageType
#undef SRK_GENERIC_INPUT_ENUM_ELEMENT
			{ (HIDReportUsagePageType)(std::numeric_limits<std::underlying_type_t<HIDReportUsagePageType>>::max)(), "__end"sv }
	};

	enum class HIDReportGenericDesktopPageType : uint32_t {
#define SRK_GENERIC_INPUT_ENUM_ELEMENT(a, b) a = b,
		SRK_GENERIC_INPUT_ENUM_HIDReportGenericDesktopPageType
#undef SRK_GENERIC_INPUT_ENUM_ELEMENT
	};

	inline static const std::unordered_map<HIDReportGenericDesktopPageType, std::string_view> HID_REPORT_GENERIC_DISKTOP_PAGE_TYPE_MAP = {
#define SRK_GENERIC_INPUT_ENUM_ELEMENT(a, b) { HIDReportGenericDesktopPageType::a, #a##sv },
			SRK_GENERIC_INPUT_ENUM_HIDReportGenericDesktopPageType
#undef SRK_GENERIC_INPUT_ENUM_ELEMENT
			{ (HIDReportGenericDesktopPageType)(std::numeric_limits<std::underlying_type_t<HIDReportGenericDesktopPageType>>::max)(), "__end"sv }
	};

	enum class HIDReportConsumerPageType : uint32_t {
#define SRK_GENERIC_INPUT_ENUM_ELEMENT(a, b) a = b,
		SRK_GENERIC_INPUT_ENUM_HIDReportConsumerPageType
#undef SRK_GENERIC_INPUT_ENUM_ELEMENT
	};

	inline static const std::unordered_map<HIDReportConsumerPageType, std::string_view> HID_REPORT_CONSUMER_PAGE_TYPE_MAP = {
#define SRK_GENERIC_INPUT_ENUM_ELEMENT(a, b) { HIDReportConsumerPageType::a, #a##sv },
			SRK_GENERIC_INPUT_ENUM_HIDReportConsumerPageType
#undef SRK_GENERIC_INPUT_ENUM_ELEMENT
			{ (HIDReportConsumerPageType)(std::numeric_limits<std::underlying_type_t<HIDReportConsumerPageType>>::max)(), "__end"sv }
	};

	enum class HIDReportLocalItemTag : uint8_t {
#define SRK_GENERIC_INPUT_ENUM_ELEMENT(a, b) a = b,
		SRK_GENERIC_INPUT_ENUM_HIDReportLocalItemTag
#undef SRK_GENERIC_INPUT_ENUM_ELEMENT
	};

	inline static const std::unordered_map<HIDReportLocalItemTag, std::string_view> HID_REPORT_LOCAL_ITEM_TAG_MAP = {
#define SRK_GENERIC_INPUT_ENUM_ELEMENT(a, b) { HIDReportLocalItemTag::a, #a##sv },
			SRK_GENERIC_INPUT_ENUM_HIDReportLocalItemTag
#undef SRK_GENERIC_INPUT_ENUM_ELEMENT
			{ (HIDReportLocalItemTag)(std::numeric_limits<std::underlying_type_t<HIDReportLocalItemTag>>::max)(), "__end"sv }
	};

	enum class HIDReportCollectionData : uint16_t {
#define SRK_GENERIC_INPUT_ENUM_ELEMENT(a, b) a = b,
		SRK_GENERIC_INPUT_ENUM_HIDReportCollectionData
#undef SRK_GENERIC_INPUT_ENUM_ELEMENT
	};

	inline static const std::unordered_map<HIDReportCollectionData, std::string_view> HID_REPORT_COLLECTION_DATA_MAP = {
#define SRK_GENERIC_INPUT_ENUM_ELEMENT(a, b) { HIDReportCollectionData::a, #a##sv },
			SRK_GENERIC_INPUT_ENUM_HIDReportCollectionData
#undef SRK_GENERIC_INPUT_ENUM_ELEMENT
			{ (HIDReportCollectionData)(std::numeric_limits<std::underlying_type_t<HIDReportCollectionData>>::max)(), "__end"sv }
	};
}
*/