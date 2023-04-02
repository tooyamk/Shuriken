#include "windows/HIDImpl.h"

#if SRK_OS == SRK_OS_WINDOWS
#include "srk/ScopeGuard.h"
#include "srk/Printer.h"
#include "srk/String.h"

#include <SetupAPI.h>

namespace srk::extensions {

	HIDDeviceInfo::HIDDeviceInfo() :
		_vendorID(0),
		_productID(0),
		_usagePage(0),
		_usage(0),
		handle(nullptr) {
	}

	HIDDeviceInfo::~HIDDeviceInfo() {
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
		do {
			if (!HidD_GetPreparsedData(handle, &preparsedData)) break;;

			HIDP_CAPS caps;
			if (HidP_GetCaps(preparsedData, &caps) != HIDP_STATUS_SUCCESS) break;

			_usagePage = caps.UsagePage;
			_usage = caps.Usage;
		} while (false);

		if (preparsedData) HidD_FreePreparsedData(preparsedData);
	}


	HIDDevice::HIDDevice(HANDLE handle, PHIDP_PREPARSED_DATA preparsedData) :
		inputReportLength(0),
		outputReportLength(0),
		featureReportLength(0),
		inputBuffer(nullptr),
		outputBuffer(nullptr),
		readPending(false),
		writePending(false),
		handle(handle),
		preparsedData(preparsedData) {
		memset(&oRead, 0, sizeof(oRead));
		memset(&oWrite, 0, sizeof(oWrite));
	}

	HIDDevice::~HIDDevice() {
		if (inputBuffer) {
			CloseHandle(oRead.hEvent);
			delete[] inputBuffer;

		}
		if (outputBuffer) {
			CloseHandle(oWrite.hEvent);
			delete[] outputBuffer;
		}

		HidD_FreePreparsedData(preparsedData);
	}

	void HIDDevice::init() {
		if (inputReportLength) {
			inputBuffer = new uint8_t[inputReportLength];
			oRead.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
		}
		if (outputReportLength) {
			outputBuffer = new uint8_t[outputReportLength];
			oWrite.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
		}
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

		for (DWORD i = 0; SetupDiEnumDeviceInterfaces(hDevInfo, nullptr, &guid, i, &deviceInterfaceData) != 0; ++i) {
			DWORD requiredSize = 0;

			if (SetupDiGetDeviceInterfaceDetailA(hDevInfo, &deviceInterfaceData, nullptr, 0, &requiredSize, nullptr); requiredSize == 0) continue;

			if (mallocDetailSize < requiredSize) {
				if (detail) free(detail);

				mallocDetailSize = requiredSize;
				if (detail = (PSP_INTERFACE_DEVICE_DETAIL_DATA_A)malloc(requiredSize); !detail) break;

				detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A);
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
			auto isContinue = callback(info, custom);

			CloseHandle(handle);

			if (!isContinue) break;
		}

		if (detail) free(detail);

		SetupDiDestroyDeviceInfoList(hDevInfo);
	}

	bool HID::isValid(const HIDDeviceInfo& info) {
		return info.handle;
	}

	bool HID::isValid(const HIDDevice& device) {
		return device.handle;
	}

	uint16_t HID::getVendorID(const HIDDeviceInfo& info) {
		return info.handle ? info.getVendorID() : 0;
	}

	uint16_t HID::getProductID(const HIDDeviceInfo& info) {
		return info.handle ? info.getProductID() : 0;
	}

	std::string_view HID::getManufacturerString(const HIDDeviceInfo& info) {
		if (info.manufacturer.empty()) {
			if (!info.handle) return std::string_view();

			WCHAR buf[256];
			if (!HidD_GetManufacturerString(info.handle, buf, sizeof(buf))) return std::string_view();
			info.manufacturer = String::wideToUtf8<std::string>(buf);
		}
		return info.manufacturer;
	}

	std::string_view HID::getProductString(const HIDDeviceInfo& info) {
		if (info.product.empty()) {
			if (!info.handle) return std::string_view();

			WCHAR buf[256];
			if (!HidD_GetProductString(info.handle, buf, sizeof(buf))) return std::string_view();
			info.product = String::wideToUtf8<std::string>(buf);
		}

		return info.product;
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

	int32_t HID::getIndex(const HIDDeviceInfo& info) {
		return info.handle ? 0 : -1;
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

		preparsedDataGuard.dismiss();

		auto dev = new HIDDevice(handle, preparsedData);
		dev->inputReportLength = caps.InputReportByteLength;
		dev->outputReportLength = caps.OutputReportByteLength;
		dev->featureReportLength = caps.FeatureReportByteLength;
		dev->init();

		return dev;
	}

	void HID::close(HIDDevice& device) {
		if (device.handle) CloseHandle(device.handle);
		delete& device;
	}

	ByteArray HID::getReportDescriptor(const HIDDevice& device) {
		using namespace srk::enum_operators;

		auto pd = device.preparsedData;
		if (!pd) return ByteArray();

		auto& header = *(const PreparsedDataHeader*)pd;
		auto items = (const PreparsedDataItem*)((const uint8_t*)pd + sizeof(PreparsedDataHeader));

		ByteArray ba;

		uint16_t usagePage = header.usagePage;
		uint16_t reportSize = 0, reportID = 0, reportCount = 0;
		int32_t logicalMinimum = 0, logicalMaximum = 0;
		int32_t physicalMinimum = 0, physicalMaximum = 0;
		uint32_t unit = 0, unitExponent = 0;

		HIDReportDescriptorItem::write(ba, HIDReportItemType::GLOBAL, (uint8_t)HIDReportGlobalItemTag::USAGE_PAGE, usagePage);
		HIDReportDescriptorItem::write(ba, HIDReportItemType::LOCAL, (uint8_t)HIDReportLocalItemTag::USAGE, header.usage);
		HIDReportDescriptorItem::write(ba, HIDReportItemType::MAIN, (uint8_t)HIDReportMainItemTag::COLLECTION, (uint8_t)HIDReportCollectionData::APPLICATION);

		auto writeItem = [&](const PreparsedDataItem& item, HIDReportMainItemTag tag) {
			if (reportID != item.reportID) {
				reportID = item.reportID;

				HIDReportDescriptorItem::write(ba, HIDReportItemType::GLOBAL, (uint8_t)HIDReportGlobalItemTag::REPORT_ID, reportID);
			}

			if (usagePage != item.usagePage) {
				usagePage = item.usagePage;

				HIDReportDescriptorItem::write(ba, HIDReportItemType::GLOBAL, (uint8_t)HIDReportGlobalItemTag::USAGE_PAGE, usagePage);
			}

			auto itemLogicalMinimum = item.logicalMinimum;
			auto itemLogicalMaximum = item.logicalMaximum;
			if (itemLogicalMinimum == itemLogicalMaximum && itemLogicalMinimum == 0 && item.usagePage == HIDReportUsagePageType::BUTTON) {
				itemLogicalMaximum = itemLogicalMinimum + (1 << item.bitSize) - 1;
			}
			if (itemLogicalMaximum < itemLogicalMinimum) {
				if (item.bitSize <= 8) {
					itemLogicalMaximum = (uint8_t)itemLogicalMaximum;
				} else if (item.bitSize <= 16) {
					itemLogicalMaximum = (uint16_t)itemLogicalMaximum;
				}
			}

			if (logicalMinimum != itemLogicalMinimum) {
				logicalMinimum = itemLogicalMinimum;

				HIDReportDescriptorItem::write(ba, HIDReportItemType::GLOBAL, (uint8_t)HIDReportGlobalItemTag::LOGICAL_MINIMUM, logicalMinimum);
			}

			if (logicalMaximum != itemLogicalMaximum) {
				logicalMaximum = itemLogicalMaximum;

				HIDReportDescriptorItem::write(ba, HIDReportItemType::GLOBAL, (uint8_t)HIDReportGlobalItemTag::LOGICAL_MAXIMUM, logicalMaximum);
			}

			auto itemPhysicalMinimum = item.physicalMinimum;
			auto itemPhysicalMaximum = item.physicalMaximum;
			if (itemPhysicalMaximum < itemPhysicalMinimum) {
				if (item.bitSize <= 8) {
					itemPhysicalMaximum = (uint8_t)itemPhysicalMaximum;
				} else if (item.bitSize <= 16) {
					itemPhysicalMaximum = (uint16_t)itemPhysicalMaximum;
				}
			}

			if (itemPhysicalMinimum != 0 || itemPhysicalMaximum != 0 || itemPhysicalMinimum != itemPhysicalMaximum) {
				if (physicalMinimum != itemPhysicalMinimum) {
					physicalMinimum = itemPhysicalMinimum;

					HIDReportDescriptorItem::write(ba, HIDReportItemType::GLOBAL, (uint8_t)HIDReportGlobalItemTag::PHYSICAL_MINIMUM, physicalMinimum);
				}

				if (physicalMaximum != itemPhysicalMaximum) {
					physicalMaximum = itemPhysicalMaximum;

					HIDReportDescriptorItem::write(ba, HIDReportItemType::GLOBAL, (uint8_t)HIDReportGlobalItemTag::PHYSICAL_MAXIMUM, physicalMaximum);
				}
			}

			if (unitExponent != item.unitExponent) {
				unitExponent = item.unitExponent;

				HIDReportDescriptorItem::write(ba, HIDReportItemType::GLOBAL, (uint8_t)HIDReportGlobalItemTag::UNIT_EXPONENT, unitExponent);
			}

			if (unit != item.unit) {
				unit = item.unit;

				HIDReportDescriptorItem::write(ba, HIDReportItemType::GLOBAL, (uint8_t)HIDReportGlobalItemTag::UNIT, unit);
			}

			if (reportSize != item.bitSize) {
				reportSize = item.bitSize;

				HIDReportDescriptorItem::write(ba, HIDReportItemType::GLOBAL, (uint8_t)HIDReportGlobalItemTag::REPORT_SIZE, reportSize);
			}

			if (reportCount != item.reportCount) {
				reportCount = item.reportCount;

				HIDReportDescriptorItem::write(ba, HIDReportItemType::GLOBAL, (uint8_t)HIDReportGlobalItemTag::REPORT_COUNT, reportCount);
			}

			if (item.usageMinimum == item.usageMaximum) {
				HIDReportDescriptorItem::write(ba, HIDReportItemType::LOCAL, (uint8_t)HIDReportLocalItemTag::USAGE, item.usageMinimum);
			} else {
				HIDReportDescriptorItem::write(ba, HIDReportItemType::LOCAL, (uint8_t)HIDReportLocalItemTag::USAGE_MINIMUM, item.usageMinimum);
				HIDReportDescriptorItem::write(ba, HIDReportItemType::LOCAL, (uint8_t)HIDReportLocalItemTag::USAGE_MAXIMUM, item.usageMaximum);
			}

			if (item.designatorMinimum != 0 || item.designatorMaximum != 0 || item.designatorMinimum != item.designatorMaximum) {
				HIDReportDescriptorItem::write(ba, HIDReportItemType::LOCAL, (uint8_t)HIDReportLocalItemTag::DESIGNATOR_MINIMUM, item.designatorMinimum);
				HIDReportDescriptorItem::write(ba, HIDReportItemType::LOCAL, (uint8_t)HIDReportLocalItemTag::DESIGNATOR_MAXIMUM, item.designatorMaximum);
			}

			if (item.stringMinimum != 0 || item.stringMaximum != 0 || item.stringMinimum != item.stringMaximum) {
				HIDReportDescriptorItem::write(ba, HIDReportItemType::LOCAL, (uint8_t)HIDReportLocalItemTag::STRING_MINIMUM, item.stringMinimum);
				HIDReportDescriptorItem::write(ba, HIDReportItemType::LOCAL, (uint8_t)HIDReportLocalItemTag::STRING_MAXIMUM, item.stringMaximum);
			}

			switch (tag) {
			case HIDReportMainItemTag::INPUT:
			{
				HIDReportDescriptorItem::write(ba, HIDReportItemType::MAIN, (uint8_t)HIDReportMainItemTag::INPUT, item.bitField);
				break;
			}
			case HIDReportMainItemTag::OUTPUT:
			{
				HIDReportDescriptorItem::write(ba, HIDReportItemType::MAIN, (uint8_t)HIDReportMainItemTag::OUTPUT, item.bitField);
				break;
			}
			case HIDReportMainItemTag::FEATURE:
			{
				HIDReportDescriptorItem::write(ba, HIDReportItemType::MAIN, (uint8_t)HIDReportMainItemTag::FEATURE, item.bitField);
				break;
			}
			default:
				break;//error
			}
		};

		std::vector<uint32_t> indices(std::max(std::max(header.inputItemCount, header.outputItemCount), header.featureItemCount));

		auto sortFn = [&](size_t l, size_t r, size_t offset) {
			const auto& li = items[l + offset];
			const auto& ri = items[r + offset];
			if (li.byteIndex < ri.byteIndex) {
				return true;
			} else if (li.byteIndex > ri.byteIndex) {
				return false;
			} else {
				return li.bitIndex < ri.bitIndex;
			}
		};

		for (decltype(header.inputItemCount) i = 0; i < header.inputItemCount; ++i) indices[i] = i;
		std::sort(indices.begin(), indices.begin() + header.inputItemCount, [&](size_t l, size_t r) {
			return sortFn(l, r, 0);
		});
		for (decltype(header.inputItemCount) i = 0; i < header.inputItemCount; ++i) writeItem(items[indices[i]], HIDReportMainItemTag::INPUT);

		for (decltype(header.outputItemCount) i = 0; i < header.outputItemCount; ++i) indices[i] = i;
		std::sort(indices.begin(), indices.begin() + header.outputItemCount, [&](size_t l, size_t r) {
			return sortFn(l, r, header.inputItemCount);
		});
		for (decltype(header.outputItemCount) i = 0; i < header.outputItemCount; ++i) writeItem(items[indices[i] + header.inputItemCount], HIDReportMainItemTag::OUTPUT);

		for (decltype(header.featureItemCount) i = 0; i < header.featureItemCount; ++i) indices[i] = i;
		std::sort(indices.begin(), indices.begin() + header.featureItemCount, [&](size_t l, size_t r) {
			return sortFn(l, r, header.inputItemCount + header.outputItemCount);
		});
		for (decltype(header.featureItemCount) i = 0; i < header.featureItemCount; ++i) writeItem(items[indices[i] + header.inputItemCount + header.outputItemCount], HIDReportMainItemTag::FEATURE);

		HIDReportDescriptorItem::write(ba, HIDReportItemType::MAIN, (uint8_t)HIDReportMainItemTag::END_COLLECTION);
		ba.seekBegin();

		//printaln(HIDReportDescriptor::toString(ba.getSource(), ba.getLength()));

		return std::move(ba);
	}

	size_t HID::read(HIDDevice& device, void* data, size_t dataLength, size_t timeout) {
		if (!device.handle) return HID::OUT_ERROR;
		if (device.inputReportLength == 0) return 0;

		DWORD bytesReaded = 0;
		auto overlapped = false;

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

		//auto src = device.inputBuffer;
		//if (bytesReaded > 0 && device.inputBuffer[0] == 0) {
		//	--bytesReaded;
		//	++src;
		//}

		auto n = dataLength > bytesReaded ? bytesReaded : dataLength;
		memcpy(data, device.inputBuffer, n);

		return n;
	}

	size_t HID::write(HIDDevice& device, const void* data, size_t dataLength, size_t timeout) {
		if (!device.handle) return HID::OUT_ERROR;
		if (device.outputReportLength == 0) return 0;

		DWORD bytesWriten = 0;
		auto overlapped = false;

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
				bytesWriten = n;
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
				bytesWriten = n;
				device.writePending = false;
			} else {
				if (GetLastError() != ERROR_IO_INCOMPLETE) {
					CancelIo(device.handle);
					device.writePending = false;
					return HID::OUT_ERROR;
				}
			}
		}

		return device.writePending ? HID::OUT_WAITTING : bytesWriten;
	}
}
#endif