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

	private:
		mutable uint16_t _vendorID;
		mutable uint16_t _productID;

		void AE_CALL _readAttrubutes() const;
	};


	class HIDDevice {
	public:
		HIDDevice(HANDLE handle);
		~HIDDevice();

		HANDLE handle;

		USHORT inputReportLength;
		USHORT outputReportLength;
		USHORT featureReportLength;

		uint8_t* readBuffer;

		bool readPending;
		OVERLAPPED oRead;
		OVERLAPPED oWrite;
	};
}
#endif