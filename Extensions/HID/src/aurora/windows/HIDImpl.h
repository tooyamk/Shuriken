#if AE_OS == AE_OS_WIN
#include "HID.h"
#include <hidsdi.h>
#include <SetupAPI.h>

namespace aurora::extensions {
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