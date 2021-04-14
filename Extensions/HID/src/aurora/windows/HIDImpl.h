#pragma once

#include "HID.h"

#if AE_OS == AE_OS_WIN
#include <optional>

#include <hidsdi.h>
#include <SetupAPI.h>

namespace aurora::extensions {
	namespace hid {
		using namespace std::literals;

		inline static const std::unordered_map<HIDReportItemType, std::string_view> HID_REPORT_ITEM_TYPE_MAP = {
#define AE_EXTENSIONS_HID_ENUM_ELEMENT(a, b) { HIDReportItemType::a, #a##sv },
				AE_EXTENSIONS_HID_ENUM_HIDReportItemType
#undef AE_EXTENSIONS_HID_ENUM_ELEMENT
				{ (HIDReportItemType)(std::numeric_limits<std::underlying_type_t<HIDReportItemType>>::max)(), "__end"sv }
		};

		inline static const std::unordered_map<HIDReportMainItemTag, std::string_view> HID_REPORT_MAIN_ITEM_TAG_MAP = {
#define AE_EXTENSIONS_HID_ENUM_ELEMENT(a, b) { HIDReportMainItemTag::a, #a##sv },
				AE_EXTENSIONS_HID_ENUM_HIDReportMainItemTag
#undef AE_EXTENSIONS_HID_ENUM_ELEMENT
				{ (HIDReportMainItemTag)(std::numeric_limits<std::underlying_type_t<HIDReportMainItemTag>>::max)(), "__end"sv }
		};

		inline static const std::unordered_map<HIDReportGlobalItemTag, std::string_view> HID_REPORT_GLOBAL_ITEM_TAG_MAP = {
#define AE_EXTENSIONS_HID_ENUM_ELEMENT(a, b) { HIDReportGlobalItemTag::a, #a##sv },
				AE_EXTENSIONS_HID_ENUM_HIDReportGlobalItemTag
#undef AE_EXTENSIONS_HID_ENUM_ELEMENT
				{ (HIDReportGlobalItemTag)(std::numeric_limits<std::underlying_type_t<HIDReportGlobalItemTag>>::max)(), "__end"sv }
		};

		inline static const std::unordered_map<HIDReportUsagePageType, std::string_view> HID_REPORT_USAGE_PAGE_TYPE_MAP = {
#define AE_EXTENSIONS_HID_ENUM_ELEMENT(a, b) { HIDReportUsagePageType::a, #a##sv },
				AE_EXTENSIONS_HID_ENUM_HIDReportUsagePageType
#undef AE_EXTENSIONS_HID_ENUM_ELEMENT
				{ (HIDReportUsagePageType)(std::numeric_limits<std::underlying_type_t<HIDReportUsagePageType>>::max)(), "__end"sv }
		};

		inline static const std::unordered_map<HIDReportGenericDesktopPageType, std::string_view> HID_REPORT_GENERIC_DISKTOP_PAGE_TYPE_MAP = {
#define AE_EXTENSIONS_HID_ENUM_ELEMENT(a, b) { HIDReportGenericDesktopPageType::a, #a##sv },
				AE_EXTENSIONS_HID_ENUM_HIDReportGenericDesktopPageType
#undef AE_EXTENSIONS_HID_ENUM_ELEMENT
				{ (HIDReportGenericDesktopPageType)(std::numeric_limits<std::underlying_type_t<HIDReportGenericDesktopPageType>>::max)(), "__end"sv }
		};

		inline static const std::unordered_map<HIDReportConsumerPageType, std::string_view> HID_REPORT_CONSUMER_PAGE_TYPE_MAP = {
#define AE_EXTENSIONS_HID_ENUM_ELEMENT(a, b) { HIDReportConsumerPageType::a, #a##sv },
				AE_EXTENSIONS_HID_ENUM_HIDReportConsumerPageType
#undef AE_EXTENSIONS_HID_ENUM_ELEMENT
				{ (HIDReportConsumerPageType)(std::numeric_limits<std::underlying_type_t<HIDReportConsumerPageType>>::max)(), "__end"sv }
		};

		inline static const std::unordered_map<HIDReportLocalItemTag, std::string_view> HID_REPORT_LOCAL_ITEM_TAG_MAP = {
#define AE_EXTENSIONS_HID_ENUM_ELEMENT(a, b) { HIDReportLocalItemTag::a, #a##sv },
				AE_EXTENSIONS_HID_ENUM_HIDReportLocalItemTag
#undef AE_EXTENSIONS_HID_ENUM_ELEMENT
				{ (HIDReportLocalItemTag)(std::numeric_limits<std::underlying_type_t<HIDReportLocalItemTag>>::max)(), "__end"sv }
		};

		inline static const std::unordered_map<HIDReportCollectionData, std::string_view> HID_REPORT_COLLECTION_DATA_MAP = {
#define AE_EXTENSIONS_HID_ENUM_ELEMENT(a, b) { HIDReportCollectionData::a, #a##sv },
				AE_EXTENSIONS_HID_ENUM_HIDReportCollectionData
#undef AE_EXTENSIONS_HID_ENUM_ELEMENT
				{ (HIDReportCollectionData)(std::numeric_limits<std::underlying_type_t<HIDReportCollectionData>>::max)(), "__end"sv }
		};


		struct ReportScopeValues {
			std::unordered_map<uint32_t, uint32_t> values;

			inline void AE_CALL set(HIDReportGlobalItemTag tag, uint32_t val) {
				values.insert_or_assign(_genKey(tag), val);
			}

			inline void AE_CALL set(HIDReportLocalItemTag tag, uint32_t val) {
				values.insert_or_assign(_genKey(tag), val);
			}

			inline std::optional<uint32_t> AE_CALL get(HIDReportGlobalItemTag tag) const {
				return _get(_genKey(tag));
			}

			inline std::optional<uint32_t> AE_CALL get(HIDReportLocalItemTag tag) const {
				return _get(_genKey(tag));
			}

			void AE_CALL clearLocal() {
				for (auto itr = values.begin(); itr != values.end();) {
					if (itr->first <= 255) {
						itr = values.erase(itr);
					} else {
						++itr;
					}
				}
			}

		private:
			inline static uint32_t AE_CALL _genKey(HIDReportGlobalItemTag tag) {
				return (uint32_t)tag << 16 | 0xFF00;
			}

			inline static uint32_t AE_CALL _genKey(HIDReportLocalItemTag tag) {
				return (uint32_t)tag;
			}

			std::optional<uint32_t> AE_CALL _get(uint16_t key) const {
				auto itr = values.find(key);
				return itr == values.end() ? std::nullopt : std::optional(itr->second);
			}
		};


		struct ReportDataInfo {
			uint16_t index;
			uint16_t length;
			bool isButton;
			void* data;
		};


		struct ReportData {
			~ReportData() {
				if (buttonCaps) delete[] buttonCaps;
				if (valueCaps) delete[] valueCaps;
			}

			USHORT numberButtonCaps;
			USHORT numberValueCaps;
			HIDP_BUTTON_CAPS* buttonCaps = nullptr;
			HIDP_VALUE_CAPS* valueCaps = nullptr;

			std::vector<ReportDataInfo> infos;
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
		};
	}


	class HIDDeviceInfo {
	public:
		HIDDeviceInfo();

		std::string_view pathView;
		HANDLE handle;

		uint16_t AE_CALL getVendorID() const;
		uint16_t AE_CALL getProductID() const;
		uint16_t AE_CALL getUsagePage() const;
		uint16_t AE_CALL getUsage() const;

	private:
		mutable uint16_t _vendorID;
		mutable uint16_t _productID;
		mutable uint16_t _usagePage;
		mutable uint16_t _usage;

		void AE_CALL _readAttrubutes() const;
		void AE_CALL _readCaps() const;
	};


	class HIDDevice {
	public:
		HIDDevice(HANDLE handle);
		~HIDDevice();

		HANDLE handle;

		USHORT inputReportLength;
		USHORT outputReportLength;
		USHORT featureReportLength;

		uint8_t* inputBuffer;
		uint8_t* outputBuffer;

		bool readPending;
		bool writePending;
		OVERLAPPED oRead;
		OVERLAPPED oWrite;
	};
}
#endif