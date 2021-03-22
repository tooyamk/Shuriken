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
		enum class HIDReportItemType {
			MAIN,
			GLOBAL,
			LOCAL
		};

		enum class HIDReportMainItemTag {
			INPUT = 0x8,
			OUTPUT = 0x9,
			COLLECTION = 0xA,
			FEATURE = 0xB,
			END_COLLECTION = 0xC,

			BEGIN = INPUT,
			END = END_COLLECTION
		};

		enum class HIDReportGlobalItemTag {
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

			BEGIN = USAGE_PAGE,
			END = REPORT_COUNT
		};

		enum class HIDReportLocalItemTag {
			USAGE = 0x0,
			USAGE_MINIMUM = 0x1,
			USAGE_MAXIMUM = 0x2,

			BEGIN = USAGE,
			END = USAGE_MAXIMUM
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