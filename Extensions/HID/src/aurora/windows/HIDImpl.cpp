#include "windows/HIDImpl.h"

#if AE_OS == AE_OS_WIN
#include "aurora/ScopeGuard.h"

namespace aurora::extensions {
	HIDDeviceInfo::HIDDeviceInfo() :
		_vendorID(0),
		_productID(0),
		_usagePage(0),
		_usage(0),
		handle(nullptr) {
	}

	uint16_t HIDDeviceInfo::getVendorID() const {
		if (!_vendorID) _readAttrubutes();
		return _vendorID;
	}

	uint16_t HIDDeviceInfo::getProductID() const {
		if (!_vendorID) _readAttrubutes();
		return _productID;
	}

	uint16_t HIDDeviceInfo::getUsagePage() const {
		if (!_usagePage) _readCaps();
		return _usagePage;
	}

	uint16_t HIDDeviceInfo::getUsage() const {
		if (!_usage) _readCaps();
		return _usage;
	}

	void HIDDeviceInfo::_readAttrubutes() const {
		if (!handle) return;

		HIDD_ATTRIBUTES attrib;
		attrib.Size = sizeof(HIDD_ATTRIBUTES);

		if (HidD_GetAttributes(handle, &attrib)) {
			_vendorID = attrib.VendorID;
			_productID = attrib.ProductID;
		}
	}

	void HIDDeviceInfo::_readCaps() const {
		if (!handle) return;

		PHIDP_PREPARSED_DATA preparsedData = nullptr;
		ScopeGuard preparsedDataGuard([&preparsedData]() {
			if (preparsedData) HidD_FreePreparsedData(preparsedData);
		});
		if (!HidD_GetPreparsedData(handle, &preparsedData)) return;

		HIDP_CAPS caps;
		if (HidP_GetCaps(preparsedData, &caps) != HIDP_STATUS_SUCCESS) return;

		_usagePage = caps.UsagePage;
		_usage = caps.Usage;
	}


	HIDDevice::HIDDevice(HANDLE handle) :
		handle(handle),
		inputReportLength(0),
		outputReportLength(0),
		featureReportLength(0),
		inputBuffer(nullptr),
		outputBuffer(nullptr),
		readPending(false),
		writePending(false) {
		memset(&oRead, 0, sizeof(oRead));
		oRead.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
		memset(&oWrite, 0, sizeof(oWrite));
		oWrite.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	}

	HIDDevice::~HIDDevice() {
		CloseHandle(oRead.hEvent);
		CloseHandle(oWrite.hEvent);
		if (inputBuffer) delete[] inputBuffer;
		if (outputBuffer) delete[] outputBuffer;
	}


	void HID::enumDevices(void* custom, HID::EnumDevicesCallback callback) {
		if (!callback) return;

		::GUID guid;
		HidD_GetHidGuid(&guid);

		auto hDevInfo = SetupDiGetClassDevsW(&guid, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
		if (!hDevInfo) return;

		SP_DEVINFO_DATA devinfoData;
		memset(&devinfoData, 0, sizeof(devinfoData));
		devinfoData.cbSize = sizeof(SP_DEVINFO_DATA);

		SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
		deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

		char driverName[256];

		HIDD_ATTRIBUTES attributes;
		attributes.Size = sizeof(HIDD_ATTRIBUTES);

		PSP_DEVICE_INTERFACE_DETAIL_DATA_A detail = nullptr;
		size_t mallocDetailSize = 0;

		PHIDP_LINK_COLLECTION_NODE linkCollectionNodes = nullptr;
		size_t numAllocatedLinkCollectionNodes = 0;

		PHIDP_BUTTON_CAPS buttonCaps = nullptr;
		size_t numAllocatedButtonCaps = 0;

		for (int32_t i = 0; SetupDiEnumDeviceInterfaces(hDevInfo, nullptr, &guid, i, &deviceInterfaceData) != 0; ++i) {
			PHIDP_PREPARSED_DATA preparsedData = nullptr;
			ScopeGuard preparsedDataGuard([&]() {
				if (preparsedData) HidD_FreePreparsedData(preparsedData);
			});
			DWORD requiredSize = 0;

			SetupDiGetDeviceInterfaceDetailA(hDevInfo, &deviceInterfaceData, nullptr, 0, &requiredSize, nullptr);
			if (requiredSize == 0) continue;

			if (mallocDetailSize < requiredSize) {
				if (detail) free(detail);

				mallocDetailSize = requiredSize;
				detail = (PSP_INTERFACE_DEVICE_DETAIL_DATA_A)malloc(requiredSize);
				if (!detail) continue;

				detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			}

			if (!SetupDiGetDeviceInterfaceDetailA(hDevInfo, &deviceInterfaceData, detail, requiredSize, nullptr, nullptr)) continue;

			if (!SetupDiEnumDeviceInfo(hDevInfo, i, &devinfoData)) continue;

			if (!SetupDiGetDeviceRegistryPropertyA(hDevInfo, &devinfoData, SPDRP_DRIVER, nullptr, (PBYTE)driverName, sizeof(driverName), nullptr)) continue;

			auto handle = CreateFileA(detail->DevicePath,
				0,
				//GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				nullptr,
				OPEN_EXISTING,
				FILE_FLAG_OVERLAPPED,
				nullptr);
			if (handle == INVALID_HANDLE_VALUE) continue;

			HIDDeviceInfo info;
			info.handle = handle;
			info.pathView = detail->DevicePath;
			callback(info, custom);

			CloseHandle(handle);
		}

		if (detail) free(detail);
		if (linkCollectionNodes) delete[] linkCollectionNodes;
		if (buttonCaps) delete[] buttonCaps;

		SetupDiDestroyDeviceInfoList(hDevInfo);
	}

	bool HID::isValid(const HIDDeviceInfo& info) {
		return info.handle;
	}

	bool HID::isValid(const HIDDevice& device) {
		return device.handle;
	}

	uint16_t HID::getVendorID(const HIDDeviceInfo& info) {
		if (!info.handle) return 0;

		return info.getVendorID();
	}

	uint16_t HID::getProductID(const HIDDeviceInfo& info) {
		if (!info.handle) return 0;

		return info.getProductID();
	}

	std::wstring HID::getManufacturerString(const HIDDeviceInfo& info) {
		if (!info.handle) return std::wstring();

		WCHAR buf[256];
		if (!HidD_GetManufacturerString(info.handle, buf, sizeof(buf))) return std::wstring();
		return std::wstring(buf);
	}

	std::wstring HID::getProductString(const HIDDeviceInfo& info) {
		if (!info.handle) return std::wstring();

		WCHAR buf[256];
		if (!HidD_GetProductString(info.handle, buf, sizeof(buf))) return std::wstring();
		return std::wstring(buf);
	}

	std::string_view HID::getPath(const HIDDeviceInfo& info) {
		return info.pathView;
	}

	uint16_t HID::getUsagePage(const HIDDeviceInfo& info) {
		return info.getUsagePage();
	}

	uint16_t HID::getUsage(const HIDDeviceInfo& info) {
		return info.getUsage();
	}

	HIDDevice* HID::open(const std::string_view& path) {
		auto handle = CreateFileA(path.data(),
			//0,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			nullptr,
			OPEN_EXISTING,
			FILE_FLAG_OVERLAPPED,
			nullptr);
		if (handle == INVALID_HANDLE_VALUE) return nullptr;

		PHIDP_PREPARSED_DATA preparsedData = nullptr;
		ScopeGuard preparsedDataGuard([&preparsedData]() {
			if (preparsedData) HidD_FreePreparsedData(preparsedData);
		});
		if (!HidD_GetPreparsedData(handle, &preparsedData)) return nullptr;

		HIDP_CAPS caps;
		if (HidP_GetCaps(preparsedData, &caps) != HIDP_STATUS_SUCCESS) return nullptr;

		auto dev = new HIDDevice(handle);
		dev->inputReportLength = caps.InputReportByteLength;
		dev->outputReportLength = caps.OutputReportByteLength;
		dev->featureReportLength = caps.FeatureReportByteLength;
		dev->inputBuffer = new uint8_t[dev->inputReportLength];
		dev->outputBuffer = new uint8_t[dev->outputReportLength];

		return dev;
	}

	void HID::close(HIDDevice& device) {
		if (device.handle) CloseHandle(device.handle);
		delete &device;
	}

	size_t HID::read(HIDDevice& device, void* data, size_t dataLength, size_t timeout) {
		if (!device.handle) return HID::OUT_ERROR;
		if (device.inputReportLength == 0) return 0;

		DWORD bytesReaded = 0;
		bool overlapped = false;

		if (device.readPending) {
			overlapped = true;
		} else {
			device.readPending = true;
			DWORD n;
			//memset(device.inputBuffer, 0, device.inputReportLength);
			ResetEvent(device.oRead.hEvent);
			if (ReadFile(device.handle, device.inputBuffer, (DWORD)device.inputReportLength, &n, &device.oRead)) {
				bytesReaded = n;
				device.readPending = false;
			} else {
				if (GetLastError() == ERROR_IO_PENDING) {
					overlapped = true;
				} else {
					CancelIo(device.handle);
					device.readPending = false;
					return HID::OUT_ERROR;
				}
			}
		}

		if (overlapped) {
			auto blocking = timeout == HID::IN_TIMEOUT_BLOCKING;
			if (timeout && !blocking) {
				if (WaitForSingleObject(device.oRead.hEvent, timeout) != WAIT_OBJECT_0) return HID::OUT_WAITTING;
			}

			DWORD n;
			if (GetOverlappedResult(device.handle, &device.oRead, &n, blocking)) { 
				bytesReaded = n;
				device.readPending = false;
			} else {
				if (GetLastError() != ERROR_IO_INCOMPLETE) {
					CancelIo(device.handle);
					device.readPending = false;
					return HID::OUT_ERROR;
				}
			}
		}

		if (device.readPending) return HID::OUT_WAITTING;

		auto src = device.inputBuffer;
		if (bytesReaded > 0 && device.inputBuffer[0] == 0) {
			--bytesReaded;
			++src;
		}

		auto n = dataLength > bytesReaded ? bytesReaded : dataLength;
		memcpy(data, src, n);

		return n;
	}

	size_t HID::write(HIDDevice& device, void* data, size_t dataLength, size_t timeout) {
		if (!device.handle) return HID::OUT_ERROR;
		if (device.outputReportLength == 0) return 0;

		DWORD bytesWrited = 0;
		bool overlapped = false;

		if (device.writePending) {
			overlapped = true;
		} else {
			if (dataLength >= device.outputReportLength) {
				memcpy(device.outputBuffer, data, device.outputReportLength);
			} else {
				memcpy(device.outputBuffer, data, dataLength);
				memset(device.outputBuffer + dataLength, 0, device.outputReportLength - dataLength);
			}

			device.writePending = true;
			DWORD n;
			ResetEvent(device.oWrite.hEvent);
			if (WriteFile(device.handle, device.outputBuffer, device.outputReportLength, &n, &device.oWrite)) {
				bytesWrited = n;
				device.writePending = false;
			} else {
				if (GetLastError() == ERROR_IO_PENDING) {
					overlapped = true;
				} else {
					CancelIo(device.handle);
					device.writePending = false;
					return HID::OUT_ERROR;
				}
			}
		}

		if (overlapped) {
			auto blocking = timeout == HID::IN_TIMEOUT_BLOCKING;
			if (timeout && !blocking) {
				if (WaitForSingleObject(device.oWrite.hEvent, timeout) != WAIT_OBJECT_0) return HID::OUT_WAITTING;
			}

			DWORD n;
			if (GetOverlappedResult(device.handle, &device.oWrite, &n, blocking)) {
				bytesWrited = n;
				device.writePending = false;
			} else {
				if (GetLastError() != ERROR_IO_INCOMPLETE) {
					CancelIo(device.handle);
					device.writePending = false;
					return HID::OUT_ERROR;
				}
			}
		}

		if (device.writePending) return HID::OUT_WAITTING;
		return bytesWrited;
	}
}
#endif