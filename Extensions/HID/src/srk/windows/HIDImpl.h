#pragma once

#include "HID.h"

#if SRK_OS == SRK_OS_WINDOWS
#include <hidsdi.h>

namespace srk::extensions {
	namespace hid {
		using namespace std::literals;

		inline static const std::unordered_map<HIDReportItemType, std::string_view> HID_REPORT_ITEM_TYPE_MAP = {
#define SRK_EXTENSION_HID_ENUM_ELEMENT(a, b) { HIDReportItemType::a, #a##sv },
				SRK_EXTENSION_HID_ENUM_HIDReportItemType
#undef SRK_EXTENSION_HID_ENUM_ELEMENT
				{ (HIDReportItemType)(std::numeric_limits<std::underlying_type_t<HIDReportItemType>>::max)(), "__end"sv }
		};

		inline static const std::unordered_map<HIDReportMainItemTag, std::string_view> HID_REPORT_MAIN_ITEM_TAG_MAP = {
#define SRK_EXTENSION_HID_ENUM_ELEMENT(a, b) { HIDReportMainItemTag::a, #a##sv },
				SRK_EXTENSION_HID_ENUM_HIDReportMainItemTag
#undef SRK_EXTENSION_HID_ENUM_ELEMENT
				{ (HIDReportMainItemTag)(std::numeric_limits<std::underlying_type_t<HIDReportMainItemTag>>::max)(), "__end"sv }
		};

		inline static const std::unordered_map<HIDReportGlobalItemTag, std::string_view> HID_REPORT_GLOBAL_ITEM_TAG_MAP = {
#define SRK_EXTENSION_HID_ENUM_ELEMENT(a, b) { HIDReportGlobalItemTag::a, #a##sv },
				SRK_EXTENSION_HID_ENUM_HIDReportGlobalItemTag
#undef SRK_EXTENSION_HID_ENUM_ELEMENT
				{ (HIDReportGlobalItemTag)(std::numeric_limits<std::underlying_type_t<HIDReportGlobalItemTag>>::max)(), "__end"sv }
		};

		inline static const std::unordered_map<HIDReportUsagePageType, std::string_view> HID_REPORT_USAGE_PAGE_TYPE_MAP = {
#define SRK_EXTENSION_HID_ENUM_ELEMENT(a, b) { HIDReportUsagePageType::a, #a##sv },
				SRK_EXTENSION_HID_ENUM_HIDReportUsagePageType
#undef SRK_EXTENSION_HID_ENUM_ELEMENT
				{ (HIDReportUsagePageType)(std::numeric_limits<std::underlying_type_t<HIDReportUsagePageType>>::max)(), "__end"sv }
		};

		inline static const std::unordered_map<HIDReportGenericDesktopPageType, std::string_view> HID_REPORT_GENERIC_DISKTOP_PAGE_TYPE_MAP = {
#define SRK_EXTENSION_HID_ENUM_ELEMENT(a, b) { HIDReportGenericDesktopPageType::a, #a##sv },
				SRK_EXTENSION_HID_ENUM_HIDReportGenericDesktopPageType
#undef SRK_EXTENSION_HID_ENUM_ELEMENT
				{ (HIDReportGenericDesktopPageType)(std::numeric_limits<std::underlying_type_t<HIDReportGenericDesktopPageType>>::max)(), "__end"sv }
		};

		inline static const std::unordered_map<HIDReportConsumerPageType, std::string_view> HID_REPORT_CONSUMER_PAGE_TYPE_MAP = {
#define SRK_EXTENSION_HID_ENUM_ELEMENT(a, b) { HIDReportConsumerPageType::a, #a##sv },
				SRK_EXTENSION_HID_ENUM_HIDReportConsumerPageType
#undef SRK_EXTENSION_HID_ENUM_ELEMENT
				{ (HIDReportConsumerPageType)(std::numeric_limits<std::underlying_type_t<HIDReportConsumerPageType>>::max)(), "__end"sv }
		};

		inline static const std::unordered_map<HIDReportLocalItemTag, std::string_view> HID_REPORT_LOCAL_ITEM_TAG_MAP = {
#define SRK_EXTENSION_HID_ENUM_ELEMENT(a, b) { HIDReportLocalItemTag::a, #a##sv },
				SRK_EXTENSION_HID_ENUM_HIDReportLocalItemTag
#undef SRK_EXTENSION_HID_ENUM_ELEMENT
				{ (HIDReportLocalItemTag)(std::numeric_limits<std::underlying_type_t<HIDReportLocalItemTag>>::max)(), "__end"sv }
		};

		inline static const std::unordered_map<HIDReportCollectionData, std::string_view> HID_REPORT_COLLECTION_DATA_MAP = {
#define SRK_EXTENSION_HID_ENUM_ELEMENT(a, b) { HIDReportCollectionData::a, #a##sv },
				SRK_EXTENSION_HID_ENUM_HIDReportCollectionData
#undef SRK_EXTENSION_HID_ENUM_ELEMENT
				{ (HIDReportCollectionData)(std::numeric_limits<std::underlying_type_t<HIDReportCollectionData>>::max)(), "__end"sv }
		};


		/*
		struct ReportDataInfo {
			uint16_t index;
			uint16_t length;
			bool isButton;
			void* data;
		};


		struct ReportData {
			USHORT numberButtonCaps;
			USHORT numberValueCaps;
			HIDReportMainItemTag tag;
			std::vector<HIDP_BUTTON_CAPS> buttonCaps;
			std::vector<HIDP_VALUE_CAPS> valueCaps;
		};


		struct ReportInfo {
			~ReportInfo() {
				if (linkCollectionNodes) delete[] linkCollectionNodes;
			}

			PHIDP_PREPARSED_DATA preparsedData = nullptr;

			USHORT usagePage = 0;
			USHORT usage = 0;

			ULONG numberLinkCollectionNodes = 0;
			HIDP_LINK_COLLECTION_NODE* linkCollectionNodes = nullptr;

			ReportData inputData;
			ReportData outputData;
			ReportData featureData;

			HIDReportScopeValues scopeValues;
			ByteArray raw;

			template<HIDReportItemTag T>
			inline void SRK_CALL setRawItem(T tag, uint32_t val) {
				if constexpr (std::same_as<T, HIDReportMainItemTag>) {
					_setRawItem(tag, val);
				} else {
					if (scopeValues.set(tag, val)) _setRawItem(tag, val);
				}

				if constexpr (std::same_as<T, HIDReportMainItemTag>) scopeValues.clearLocal();
			}

			template<HIDReportItemType Type>
			inline void SRK_CALL setRawItem(uint8_t tag, uint32_t val) {
				if constexpr (std::same_as<Type == HIDReportItemType::MAIN>) {
					setRawItem((HIDReportMainItemTag)tag, val);
				} else if constexpr (std::same_as<Type == HIDReportItemType::GLOBAL>) {
					setRawItem((HIDReportGlobalItemTag)tag, val);
				} else if constexpr (std::same_as<Type == HIDReportItemType::LOCAL>) {
					setRawItem((HIDReportLocalItemTag)tag, val);
				}
			}

			template<HIDReportItemTag T>
			inline void SRK_CALL setRawItem(T tag) {
				raw.write<uint8_t>(HID::generateReportShortItemHeader(tag, 0));

				if constexpr (std::same_as<T, HIDReportMainItemTag>) scopeValues.clearLocal();
			}

			template<HIDReportItemType Type>
			inline void SRK_CALL setRawItem(uint8_t tag) {
				if constexpr (std::same_as<Type == HIDReportItemType::MAIN>) {
					setRawItem((HIDReportMainItemTag)tag);
				} else if constexpr (std::same_as<Type == HIDReportItemType::GLOBAL>) {
					setRawItem((HIDReportGlobalItemTag)tag);
				} else if constexpr (std::same_as<Type == HIDReportItemType::LOCAL>) {
					setRawItem((HIDReportLocalItemTag)tag);
				}
			}

		private:
			template<HIDReportItemTag T>
			void SRK_CALL _setRawItem(T tag, uint32_t val) {
				uint8_t bytes = 1;
				if (val > (std::numeric_limits<uint16_t>::max)()) {
					bytes = 4;
				} else if (val > (std::numeric_limits<uint8_t>::max)()) {
					bytes = 2;
				}

				raw.write<uint8_t>(HID::generateReportShortItemHeader(tag, bytes));
				switch (bytes) {
				case 1:
					raw.write<uint8_t>(val);
					break;
				case 2:
					raw.write<uint16_t>(val);
					break;
				case 4:
					raw.write<uint32_t>(val);
					break;
				}
			}
		};
		*/
	}


	class HIDDeviceInfo {
	public:
		HIDDeviceInfo();
		~HIDDeviceInfo();

		std::string_view pathView;
		mutable std::string manufacturer;
		mutable std::string product;
		HANDLE handle;

		uint16_t SRK_CALL getVendorID() const;
		uint16_t SRK_CALL getProductID() const;
		HIDUsagePage SRK_CALL getUsagePage() const;
		HIDUsage SRK_CALL getUsage() const;
		void* SRK_CALL getPreparsedData() const;
		//ByteArray SRK_CALL getRawReportDescriptor() const;

	protected:
		mutable uint16_t _vendorID;
		mutable uint16_t _productID;
		mutable HIDUsagePage _usagePage;
		mutable HIDUsage _usage;
		mutable ByteArray _rawReportDescriptor;

		mutable PHIDP_PREPARSED_DATA _preparsedData;

		void SRK_CALL _readAttrubutes() const;
		void SRK_CALL _readCaps() const;
	};


	class HIDDevice {
	public:
		HIDDevice(HANDLE handle, PHIDP_PREPARSED_DATA preparsedData);
		~HIDDevice();

		USHORT inputReportLength;
		USHORT outputReportLength;
		USHORT featureReportLength;

		uint8_t* inputBuffer;
		uint8_t* outputBuffer;

		bool readPending;
		bool writePending;
		OVERLAPPED oRead;
		OVERLAPPED oWrite;

		HANDLE handle;
		PHIDP_PREPARSED_DATA preparsedData;

		void SRK_CALL init();
		//ULONG SRK_CALL parsePressedButtons(HIDP_REPORT_TYPE reportType, USAGE usagePage, const void* reportData, ULONG reportDataLength, USAGE* outUsages, ULONG usageLength) const;
		//std::optional<ULONG> SRK_CALL parseValue(HIDP_REPORT_TYPE reportType, USAGE usagePage, USAGE usage, const void* report, ULONG reportLength) const;
	};
}
#endif