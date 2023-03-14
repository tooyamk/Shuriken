#pragma once

#include "HID.h"

#if SRK_OS == SRK_OS_WINDOWS
#include <hidsdi.h>

namespace srk::extensions {
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

	protected:
		mutable uint16_t _vendorID;
		mutable uint16_t _productID;
		mutable HIDUsagePage _usagePage;
		mutable HIDUsage _usage;

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
	};


	//seee https://chromium.googlesource.com/chromium/src/+/73fdaaf605bb60caf34d5f30bb84a417688aa528/services/device/hid/hid_preparsed_data.cc
	struct PreparsedDataHeader {
		uint16_t magic[4];
		uint16_t usage;
		uint16_t usagePage;
		uint16_t unknown[3];
		uint16_t inputItemCount;
		uint16_t unknown2;
		uint16_t inputReportByteLength;
		uint16_t unknown3;
		uint16_t outputItemCount;
		uint16_t unknown4;
		uint16_t outputReportByteLength;
		uint16_t unknown5;
		uint16_t featureItemCount;
		uint16_t itemCount;
		uint16_t featureReportByteLength;
		uint16_t sizeBytes;
		uint16_t unknown6;
	};

	struct PreparsedDataItem {
		uint16_t usagePage;
		uint8_t reportID;
		uint8_t bitIndex;
		uint16_t bitSize;
		uint16_t reportCount;
		uint16_t byteIndex;
		uint16_t bitCount;
		uint32_t bitField;
		uint32_t unknown;
		uint16_t linkUsagePage;
		uint16_t linkUsage;
		uint32_t unknown2[9];
		uint16_t usageMinimum;
		uint16_t usageMaximum;
		uint16_t stringMinimum;
		uint16_t stringMaximum;
		uint16_t designatorMinimum;
		uint16_t designatorMaximum;
		uint16_t dataIndexMinimum;
		uint16_t dataIndexMaximum;
		uint32_t unknown3;
		int32_t logicalMinimum;
		int32_t logicalMaximum;
		int32_t physicalMinimum;
		int32_t physicalMaximum;
		uint32_t unit;
		uint32_t unitExponent;
	};
}
#endif