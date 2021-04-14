#include "windows/HIDImpl.h"

#if AE_OS == AE_OS_WIN
#include "aurora/ScopeGuard.h"
#include "aurora/Debug.h"

namespace aurora::extensions {
	namespace hid {
		inline std::string AE_CALL getUsagePageString(USHORT usagePage) {
			auto val = (HIDReportUsagePageType)usagePage;
			if (val >= HIDReportUsagePageType::POWER_PAGES_BEGIN && val <= HIDReportUsagePageType::POWER_PAGES_END) {
				return "POWER_PAGES";
			} else if (val >= HIDReportUsagePageType::VENDOR_DEFINED_BEGIN && val <= HIDReportUsagePageType::VENDOR_DEFINED_END) {
				return "VENDOR_DEFINED";
			} else {
				if (auto itr = HID_REPORT_USAGE_PAGE_TYPE_MAP.find(val); itr != HID_REPORT_USAGE_PAGE_TYPE_MAP.end()) {
					return std::string(itr->second);
				} else {
					return "RESERVED";
				}
			}
		}

		inline std::string AE_CALL getUsageString(USHORT usagePage, USHORT usage) {
			switch ((HIDReportUsagePageType)usagePage) {
			case HIDReportUsagePageType::GENERIC_DESKTOP:
			{
				auto val = (HIDReportGenericDesktopPageType)usage;
				if (auto itr = HID_REPORT_GENERIC_DISKTOP_PAGE_TYPE_MAP.find(val); itr != HID_REPORT_GENERIC_DISKTOP_PAGE_TYPE_MAP.end()) {
					return std::string(itr->second);
				} else {
					return "RESERVED";
				}

				break;
			}
			case HIDReportUsagePageType::CONSUMER_DEVICES:
			{
				auto val = (HIDReportConsumerPageType)usage;
				if (auto itr = HID_REPORT_CONSUMER_PAGE_TYPE_MAP.find(val); itr != HID_REPORT_CONSUMER_PAGE_TYPE_MAP.end()) {
					return std::string(itr->second);
				} else {
					return "RESERVED";
				}

				break;
			}
			case HIDReportUsagePageType::BUTTON:
			{
				if (usage == 0) {
					return "NO_BUTTON_PRESED";
				} else {
					return "BUTTON_" + String::toString(usage);
				}

				break;
			}
			default:
			{
				return String::toString(usage);

				break;
			}
			}
		}

		template<typename T>
		inline size_t AE_CALL findDataCaps(const T* caps, size_t n, size_t begin, USHORT collection) {
			for (; begin < n; ++begin) {
				if (caps[begin].LinkCollection == collection) return begin;
			}

			return n + 1;
		};

		inline void AE_CALL _HIDReportDataCaps(std::string& str, HIDP_BUTTON_CAPS& caps, const std::string& indent) {
			str += indent + "UsagePage (" + getUsagePageString(caps.UsagePage) + ")\n";
			if (caps.IsRange) {
				str += indent + "UsageMinimum (" + getUsageString(caps.UsagePage, caps.Range.UsageMin) + ")\n";
				str += indent + "UsageMaximum (" + getUsageString(caps.UsagePage, caps.Range.UsageMax) + ")\n";
			} else {
				str += indent + "Usage (" + getUsageString(caps.UsagePage, caps.NotRange.Usage) + ")\n";
			}
			str += indent + "ReportID (" + String::toString(caps.ReportID) + ")\n";
			str += indent + "LogicalMinimum (" + String::toString(0) + ")\n";
			str += indent + "LogicalMaximum (" + String::toString(1) + ")\n";
			str += indent + "ReportSize (" + String::toString(1) + ")\n";
			str += indent + "ReportCount (" + String::toString(caps.Range.UsageMax - caps.Range.UsageMin + 1) + ")\n\n";

			int a = 1;
		}

		inline void AE_CALL _HIDReportDataCaps(std::string& str, HIDP_VALUE_CAPS& caps, const std::string& indent) {
			str += indent + "UsagePage (" + getUsagePageString(caps.UsagePage) + ")\n";
			if (caps.IsRange) {
				str += indent + "UsageMinimum (" + getUsageString(caps.UsagePage, caps.Range.UsageMin) + ")\n";
				str += indent + "UsageMaximum (" + getUsageString(caps.UsagePage, caps.Range.UsageMax) + ")\n";
			} else {
				str += indent + "Usage (" + getUsageString(caps.UsagePage, caps.NotRange.Usage) + ")\n";
			}
			str += indent + "ReportID (" + String::toString(caps.ReportID) + ")\n";
			str += indent + "LogicalMinimum (" + String::toString(caps.LogicalMin) + ")\n";
			str += indent + "LogicalMaximum (" + String::toString(caps.LogicalMax) + ")\n";
			str += indent + "PhysicalMinimum (" + String::toString(caps.PhysicalMin) + ")\n";
			str += indent + "PhysicalMaximum (" + String::toString(caps.PhysicalMax) + ")\n";
			str += indent + "ReportSize (" + String::toString(caps.BitSize) + ")\n";
			str += indent + "ReportCount (" + String::toString(caps.ReportCount) + ")\n\n";

			int a = 1;
		}

		template<typename T>
		inline void AE_CALL _HIDReportCollectData(std::vector<ReportDataInfo>& vec, T* caps, size_t numberCaps, size_t collectionIndex) {
			size_t i = 0;
			do {
				i = findDataCaps(caps, numberCaps, i, collectionIndex);
				if (i >= numberCaps) {
					break;
				} else {
					auto& info = vec.emplace_back();

					auto& data = caps[i];
					if (data.IsRange) {
						info.index = data.Range.DataIndexMin;
						info.length = data.Range.DataIndexMax - data.Range.DataIndexMin + 1;
					} else {
						info.index = data.NotRange.DataIndex;
						info.length = 1;
					}

					info.isButton = std::same_as<std::remove_cvref_t<T>, HIDP_BUTTON_CAPS>;
					info.data = &data;

					++i;
				}
			} while (true);
		}

		inline void AE_CALL _HIDReportCollectData(std::string& str, ReportData& data, size_t collectionIndex, const std::string& indent) {
			std::vector<ReportDataInfo> vec;

			_HIDReportCollectData(vec, data.buttonCaps, data.numberButtonCaps, collectionIndex);
			_HIDReportCollectData(vec, data.valueCaps, data.numberValueCaps, collectionIndex);

			size_t index = 0;
			size_t n = vec.size();
			while (n) {
				bool found = false;
				for (auto& info : vec) {
					if (index == info.index) {
						data.infos.emplace_back(info);
						index += info.length;
						--n;
						found = true;

						break;
					}
				}

				if (!found) break;
			}

			for (auto& info : data.infos) {
				if (info.isButton) {
					_HIDReportDataCaps(str, *(HIDP_BUTTON_CAPS*)info.data, indent);
				} else {
					_HIDReportDataCaps(str, *(HIDP_VALUE_CAPS*)info.data, indent);
				}
			}
		}

		inline void AE_CALL _HIDReportCollection(std::string& str, ReportInfo& info, size_t collectionIndex, const std::string& indent) {
			auto& collection = info.linkCollectionNodes[collectionIndex];

			str += indent + "Collection (";

			auto indent1 = indent + "  ";

			auto val = (HIDReportCollectionData)collection.CollectionType;
			if (val >= HIDReportCollectionData::VENDOR_DEFINED_BEGIN && val <= HIDReportCollectionData::VENDOR_DEFINED_END) {
				str += "VENDOR_DEFINED";
			} else {
				if (auto itr = HID_REPORT_COLLECTION_DATA_MAP.find(val); itr != HID_REPORT_COLLECTION_DATA_MAP.end()) {
					str += itr->second;
				} else {
					str += "RESERVED";
				}
			}

			str += ")\n";

			_HIDReportCollectData(str, info.inputData, collectionIndex, indent1);
			_HIDReportCollectData(str, info.outputData, collectionIndex, indent1);
			_HIDReportCollectData(str, info.featureData, collectionIndex, indent1);

			auto c = collection.FirstChild;
			while (c) {
				_HIDReportCollection(str, info, c, indent1);
				c = info.linkCollectionNodes[c].NextSibling;
			}

			str += indent + "EndCollection ()\n";
		}

		inline void AE_CALL _HIDReportReadDataCaps(ReportData& data, HIDP_REPORT_TYPE reportType, ReportInfo& info) {
			data.buttonCaps = new HIDP_BUTTON_CAPS[data.numberButtonCaps];
			HidP_GetButtonCaps(HidP_Input, data.buttonCaps, &data.numberButtonCaps, info.preparsedData);

			data.valueCaps = new HIDP_VALUE_CAPS[data.numberValueCaps];
			HidP_GetValueCaps(HidP_Input, data.valueCaps, &data.numberValueCaps, info.preparsedData);
		}

		inline void AE_CALL _HIDReportDescriptor(ReportInfo& info) {
			std::string indent = "";

			std::string str;

			str += indent + "UsagePage (" + getUsagePageString(info.usagePage) + ")\n";
			str += indent + "Usage (" + getUsageString(info.usagePage, info.usage) + ")\n";

			{
				if (info.numberLinkCollectionNodes) {
					info.linkCollectionNodes = new HIDP_LINK_COLLECTION_NODE[info.numberLinkCollectionNodes];
					HidP_GetLinkCollectionNodes(info.linkCollectionNodes, &info.numberLinkCollectionNodes, info.preparsedData);

					_HIDReportReadDataCaps(info.inputData, HidP_Input, info);
					_HIDReportReadDataCaps(info.outputData, HidP_Output, info);
					_HIDReportReadDataCaps(info.featureData, HidP_Feature, info);

					for (USHORT i = 0; i < info.numberLinkCollectionNodes; ++i) {
						if (info.linkCollectionNodes[i].Parent != 0) continue;

						_HIDReportCollection(str, info, i, indent);
					}
				}
			}

			printdln(str);
		}
	}

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

		if (0) {
			hid::ReportInfo info;
			info.preparsedData = preparsedData;
			info.usagePage = caps.UsagePage;
			info.usage = caps.Usage;
			info.numberLinkCollectionNodes = caps.NumberLinkCollectionNodes;
			info.inputData.numberButtonCaps = caps.NumberInputButtonCaps;
			info.inputData.numberValueCaps = caps.NumberInputValueCaps;
			info.outputData.numberButtonCaps = caps.NumberOutputButtonCaps;
			info.outputData.numberValueCaps = caps.NumberOutputValueCaps;
			info.featureData.numberButtonCaps = caps.NumberFeatureButtonCaps;
			info.featureData.numberValueCaps = caps.NumberFeatureValueCaps;
			hid::_HIDReportDescriptor(info);
		}

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