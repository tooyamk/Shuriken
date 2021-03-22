#pragma once

#include "Base.h"
#include "aurora/events/EventDispatcher.h"

namespace aurora::modules::inputs::generic_input {
	class AE_MODULE_DLL Input : public IInputModule {
	public:
		Input(Ref* loader);
		virtual ~Input();

		void operator delete(Input* p, std::destroying_delete_t) {
			auto l = p->_loader;
			p->~Input();
			::operator delete(p);
		}

		virtual events::IEventDispatcher<ModuleEvent>& AE_CALL getEventDispatcher() override;
		virtual void AE_CALL poll() override;
		virtual IInputDevice* AE_CALL createDevice(const DeviceGUID& guid) override;

	private:
		enum class HIDReportItemType : uint8_t {
			MAIN,
			GLOBAL,
			LOCAL
		};

		enum class HIDReportMainItemTag : uint8_t {
			INPUT = 0x8,
			OUTPUT = 0x9,
			COLLECTION = 0xA,
			FEATURE = 0xB,
			END_COLLECTION = 0xC,

			BEGIN = INPUT,
			END = END_COLLECTION
		};

		enum class HIDReportGlobalItemTag : uint8_t {
			USAGE_PAGE = 0x0,
			LOGICAL_MINIMUM = 0x1,
			LOGICAL_MAXIMUM = 0x2,
			PHYSICAL_MINIMUM = 0x3,
			PHYSICAL_MAXIMUM = 0x4,
			UNIT_EXPONENT = 0x5,
			UNIT = 0x6,
			REPORT_SIZE = 0x7,
			REPORT_ID = 0x8,
			REPORT_COUNT = 0x9,
			PUSH = 0xA,
			POP = 0xB,

			BEGIN = USAGE_PAGE,
			END = POP
		};

		enum class HIDReportUsagePageType : uint16_t {
			UNDEFINED = 0x0,
			GENERIC_DESKTOP_PAGE = 0x1,
			SIMULATION_CONTROLS_PAGE = 0x2,
			VR_CONTROLS_PAGE = 0x3,
			SPORT_CONTROLS_PAGE = 0x4,
			GAME_CONTROLS_PAGE = 0x5,
			GENERIC_DEVICE_CONTROLS_PAGE = 0x6,
			KEYBOARD_OR_KEYPAD_PAGE = 0x7,
			LED_PAGE  = 0x8,
			BUTTON_PAGE = 0x9,
			ORDINAL_PAGE = 0xA,
			TELEPHONY_DEVICE_PAGE = 0xB,
			CONSUMER_PAGE = 0xC,
			DIGITIZERS_PAGE = 0xD,
			HAPTICS_PAGE = 0xE,
			PID_PAGE = 0xF,
			UNICODE_PAGE = 0x10,
			//RESERVED 0x11
			EYE_AND_HEAD_TRACKERS_PAGE = 0x12,
			//RESERVED 0x13
			AUXILIARY_DISPLAY_PAGE = 0x14,
			//RESERVED 0x15-0x1F
			SENSORS_PAGE = 0x20,
			//RESERVED 0x21-0x3F
			MEDIA_INSTRUMENT_PAGE = 0x40,
			BRAILLE_DISPLAY_PAGE = 0x41,
			//RESERVED 0x42-0x58
			LIGHTING_AND_ILLUMINATION_PAGE = 0x59,
			//RESERVED 0x5A-0x7F
			MONITOR_PAGES_BEGIN = 0x80,
			MONITOR_PAGES_END = 0x83,
			POWER_PAGES_BEGIN = 0x84,
			POWER_PAGES_END = 0x87,
			//RESERVED 0x88-0x8B
			BAR_CODE_SCANNER_PAGE = 0x8C,
			SCALE_PAGE = 0x8D,
			MAGNETIC_STRIPE_REDING_DEVICES = 0x8E,
			RESERVED_POINT_OF_SALE_PAGES = 0x8F,
			CAMERA_CONTROL_PAGE = 0x90,
			ARCADE_PAGE = 0x91,
			GANNING_DEVICE_PAGE = 0x92,
			//RESERVED 0x93-0xF1CF
			FIDO_ALLIANCE_PAGE = 0xF1D0,
			//RESERVED 0xF1D1-0xFEFF
			VENDOR_DEFINED_BEGIN = 0xFF00,
			VENDOR_DEFINED_END = 0xFFFF,

			BEGIN = UNDEFINED,
			END = VENDOR_DEFINED_END
		};

		enum class HIDReportLocalItemTag : uint8_t {
			USAGE = 0x0,
			USAGE_MINIMUM = 0x1,
			USAGE_MAXIMUM = 0x2,

			BEGIN = USAGE,
			END = USAGE_MAXIMUM
		};

		enum class HIDReportCollectionData : uint8_t {
			PHYSICAL = 0x0,
			APPLICATION = 0x1,
			LOGICAL = 0x2,
			REPORT = 0x3,
			NAMED_ARRAY = 0x4,
			USAGE_MODIFIER = 0x5,
			USAGE_SWITCH = 0x6,
			RESERVED_BEGIN = 0x7,
			RESERVED_END = 0x7F,
			VENDOR_DEFINED_BEGIN = 0x80,
			VENDOR_DEFINED_END = 0xFF,

			BEGIN = PHYSICAL,
			END = VENDOR_DEFINED_END
		};

		struct HIDDescriptor {
			struct Desc {
				uint8_t  bType;
				uint16_t wLength;
			};

			uint8_t bLength;
			uint8_t bDescriptorType;
			uint16_t bcdHID;
			uint8_t bCountry;
			uint8_t bNumDescriptors;
			Desc DescriptorList[2];

			void AE_CALL set(void* data, size_t size) {
				ByteArray ba(data, size, ByteArray::Usage::SHARED);
				bLength = ba.read<decltype(bLength)>();
				bDescriptorType = ba.read<decltype(bDescriptorType)>();
				bcdHID = ba.read<decltype(bcdHID)>();
				bCountry = ba.read<decltype(bCountry)>();
				bNumDescriptors = ba.read<decltype(bNumDescriptors)>();
				for (size_t i = 0; i < 2; ++i) {
					auto& desc = DescriptorList[i];
					desc.bType = ba.read<decltype(desc.bType)>();
					desc.wLength = ba.read<decltype(desc.wLength)>();
				}
			}
		};


		struct InternalDeviceInfo {
			int32_t readInterfaceIdx = -1, readEndpointIdx = -1, writeInterfaceIdx = -1, writeEndpointIdx = -1;
			DeviceType bestType = DeviceType::UNKNOWN;
			size_t score = 0;
		};


		IntrusivePtr<Ref> _loader;

		events::EventDispatcher<ModuleEvent> _eventDispatcher;

		std::vector<DeviceInfo> _devices;
		std::vector<DeviceInfo> _newDevices;
		std::vector<uint32_t> _keepDevices;

		libusb_context* _context;

		void AE_CALL _calcGUID(libusb_device* device, const libusb_device_descriptor& desc, DeviceGUID& guid);
		void AE_CALL _findDevices();
		void AE_CALL _checkDevice(libusb_device* device);
	};
}