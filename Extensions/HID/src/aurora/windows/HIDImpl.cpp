#include "windows/HIDImpl.h"

#if AE_OS == AE_OS_WIN
#include "aurora/ScopeGuard.h"
#include "aurora/Debug.h"

#include <SetupAPI.h>

//#include <winusb.h>
//#include <winioctl.h>
//#include <hidport.h>
//#include <hidclass.h>
//#include <winioctl.h>
//#define HID_OUT_CTL_CODE(id)  \
//		CTL_CODE(FILE_DEVICE_KEYBOARD, (id), METHOD_OUT_DIRECT,, FILE_ANY_ACCESS)
//#define IOCTL_HID_GET_FEATURE                   HID_OUT_CTL_CODE(100)
//#define IOCTL_HID_GET_INPUT_REPORT              HID_OUT_CTL_CODE(104)

#define AE_EXTENSION_HID_PRINT_ENABLED

namespace aurora::extensions {
	/*
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

		template<SameAnyOf<HIDP_BUTTON_CAPS, HIDP_VALUE_CAPS> T>
		inline void AE_CALL _HIDReportDataCaps(std::string& str, ReportInfo& info, T& caps, const std::string& indent) {
			info.setRawItem(HIDReportGlobalItemTag::REPORT_ID, caps.ReportID);
			info.setRawItem(HIDReportGlobalItemTag::USAGE_PAGE, caps.UsagePage);

#ifdef AE_EXTENSION_HID_PRINT_ENABLED
			str += indent + "UsagePage (" + getUsagePageString(caps.UsagePage) + ")(" + String::toString(caps.UsagePage) + ")\n";
#endif
			if (caps.IsRange) {
				info.setRawItem(HIDReportLocalItemTag::USAGE_MINIMUM, caps.Range.UsageMin);
				info.setRawItem(HIDReportLocalItemTag::USAGE_MAXIMUM, caps.Range.UsageMax);
#ifdef AE_EXTENSION_HID_PRINT_ENABLED
				str += indent + "UsageMinimum (" + getUsageString(caps.UsagePage, caps.Range.UsageMin) + ")(" + String::toString(caps.Range.UsageMin) + ")\n";
				str += indent + "UsageMaximum (" + getUsageString(caps.UsagePage, caps.Range.UsageMax) + ")(" + String::toString(caps.Range.UsageMax) + ")\n";
#endif
			} else {
				info.setRawItem(HIDReportLocalItemTag::USAGE, caps.NotRange.Usage);
#ifdef AE_EXTENSION_HID_PRINT_ENABLED
				str += indent + "Usage (" + getUsageString(caps.UsagePage, caps.NotRange.Usage) + ")(" + String::toString(caps.NotRange.Usage) + ")\n";
#endif
			}

#ifdef AE_EXTENSION_HID_PRINT_ENABLED
			str += indent + "ReportID (" + String::toString(caps.ReportID) + ")\n";
#endif

			if constexpr (std::same_as<T, HIDP_BUTTON_CAPS>) {
				info.setRawItem(HIDReportGlobalItemTag::LOGICAL_MINIMUM, 0);
				info.setRawItem(HIDReportGlobalItemTag::LOGICAL_MAXIMUM, 1);
				info.setRawItem(HIDReportGlobalItemTag::REPORT_SIZE, 1);
				info.setRawItem(HIDReportGlobalItemTag::REPORT_COUNT, caps.Range.UsageMax - caps.Range.UsageMin + 1);

#ifdef AE_EXTENSION_HID_PRINT_ENABLED
				str += indent + "LogicalMinimum (" + String::toString(0) + ")\n";
				str += indent + "LogicalMaximum (" + String::toString(1) + ")\n";
				str += indent + "ReportSize (" + String::toString(1) + ")\n";
				str += indent + "ReportCount (" + String::toString(caps.Range.UsageMax - caps.Range.UsageMin + 1) + ")\n";
#endif
			} else {
				info.setRawItem(HIDReportGlobalItemTag::LOGICAL_MINIMUM, caps.LogicalMin);
				info.setRawItem(HIDReportGlobalItemTag::LOGICAL_MAXIMUM, caps.LogicalMax);
				info.setRawItem(HIDReportGlobalItemTag::PHYSICAL_MINIMUM, caps.PhysicalMin);
				info.setRawItem(HIDReportGlobalItemTag::PHYSICAL_MAXIMUM, caps.PhysicalMax);
				info.setRawItem(HIDReportGlobalItemTag::REPORT_SIZE, caps.BitSize);
				info.setRawItem(HIDReportGlobalItemTag::REPORT_COUNT, caps.ReportCount);

#ifdef AE_EXTENSION_HID_PRINT_ENABLED
				str += indent + "LogicalMinimum (" + String::toString(caps.LogicalMin) + ")\n";
				str += indent + "LogicalMaximum (" + String::toString(caps.LogicalMax) + ")\n";
				str += indent + "PhysicalMinimum (" + String::toString(caps.PhysicalMin) + ")\n";
				str += indent + "PhysicalMaximum (" + String::toString(caps.PhysicalMax) + ")\n";
				str += indent + "ReportSize (" + String::toString(caps.BitSize) + ")\n";
				str += indent + "ReportCount (" + String::toString(caps.ReportCount) + ")\n";
#endif
			}
		}

		template<typename T>
		inline void AE_CALL _HIDReportCollectData(std::vector<ReportDataInfo>& vec, std::vector<T>& caps, size_t collectionIndex) {
			auto max = caps.size();
			size_t i = 0;
			do {
				i = findDataCaps(caps.data(), max, i, collectionIndex);
				if (i >= max) {
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

		inline void AE_CALL _HIDReportCollectData(std::string& str, ReportInfo& info, ReportData& data, size_t collectionIndex, const std::string& indent) {
			std::vector<ReportDataInfo> vec;

			_HIDReportCollectData(vec, data.buttonCaps, collectionIndex);
			_HIDReportCollectData(vec, data.valueCaps, collectionIndex);

			std::sort(vec.begin(), vec.end(), [](const ReportDataInfo& left, const ReportDataInfo& right) {
				return left.index < right.index;
			});

			for (auto& e : vec) {
				if (e.isButton) {
					_HIDReportDataCaps(str, info, *(HIDP_BUTTON_CAPS*)e.data, indent);
				} else {
					_HIDReportDataCaps(str, info, *(HIDP_VALUE_CAPS*)e.data, indent);
				}

				info.setRawItem(data.tag, 0b10);

#ifdef AE_EXTENSION_HID_PRINT_ENABLED
				switch (data.tag) {
				case HIDReportMainItemTag::INPUT:
					str += indent + "Input ()\n";
					break;
				case HIDReportMainItemTag::OUTPUT:
					str += indent + "Output ()\n";
					break;
				case HIDReportMainItemTag::FEATURE:
					str += indent + "Feature ()\n";
					break;
				}

				str += "\n";
#endif
			}
		}

		inline void AE_CALL _HIDReportCollection(std::string& str, ReportInfo& info, size_t collectionIndex, const std::string& indent) {
			auto& collection = info.linkCollectionNodes[collectionIndex];

			info.setRawItem(HIDReportMainItemTag::COLLECTION, collection.CollectionType);

			auto indent1 = indent + "  ";
#ifdef AE_EXTENSION_HID_PRINT_ENABLED
			str += indent + "Collection (";

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
#endif

			_HIDReportCollectData(str, info, info.inputData, collectionIndex, indent1);
			_HIDReportCollectData(str, info, info.outputData, collectionIndex, indent1);
			_HIDReportCollectData(str, info, info.featureData, collectionIndex, indent1);

			auto c = collection.FirstChild;
			while (c) {
				_HIDReportCollection(str, info, c, indent1);
				c = info.linkCollectionNodes[c].NextSibling;
			}

			info.setRawItem(HIDReportMainItemTag::END_COLLECTION);

#ifdef AE_EXTENSION_HID_PRINT_ENABLED
			str += indent + "EndCollection ()\n";
#endif
		}

		inline void AE_CALL _HIDReportReadDataCaps(ReportData& data, HIDP_REPORT_TYPE reportType, ReportInfo& info) {
			if (data.numberButtonCaps) {
				data.buttonCaps.resize(data.numberButtonCaps);
				if (HidP_GetButtonCaps(reportType, data.buttonCaps.data(), &data.numberButtonCaps, info.preparsedData) != HIDP_STATUS_SUCCESS) {
					int a = 1;
				}

				for (size_t i = 0; i < data.numberButtonCaps; ++i) {
					auto& caps = data.buttonCaps[i];
					int a = 1;
				}
			}

			if (data.numberValueCaps) {
				data.valueCaps.resize(data.numberValueCaps);
				if (HidP_GetValueCaps(reportType, data.valueCaps.data(), &data.numberValueCaps, info.preparsedData) != HIDP_STATUS_SUCCESS) {
					int a = 1;
				}

				for (size_t i = 0; i < data.numberValueCaps; ++i) {
					auto& caps = data.valueCaps[i];
					int a = 1;
				}
			}

			int a = 1;
		}

		inline void AE_CALL _HIDReportDescriptor(ReportInfo& info) {
			std::string indent = "";

			std::string str;

			info.setRawItem(HIDReportGlobalItemTag::USAGE_PAGE, info.usagePage);
			info.setRawItem(HIDReportLocalItemTag::USAGE, info.usage);

#ifdef AE_EXTENSION_HID_PRINT_ENABLED
			str += indent + "UsagePage (" + getUsagePageString(info.usagePage) + ")(" + String::toString(info.usagePage) + ")\n";
			str += indent + "Usage (" + getUsageString(info.usagePage, info.usage) + ")(" + String::toString(info.usage) + ")\n";
#endif

			{
				if (info.numberLinkCollectionNodes) {
					info.linkCollectionNodes = new HIDP_LINK_COLLECTION_NODE[info.numberLinkCollectionNodes];
					HidP_GetLinkCollectionNodes(info.linkCollectionNodes, &info.numberLinkCollectionNodes, info.preparsedData);

					_HIDReportReadDataCaps(info.inputData, HidP_Input, info);
					_HIDReportReadDataCaps(info.outputData, HidP_Output, info);
					_HIDReportReadDataCaps(info.featureData, HidP_Feature, info);

					if (info.numberLinkCollectionNodes) _HIDReportCollection(str, info, 0, indent);
				}
			}

#ifdef AE_EXTENSION_HID_PRINT_ENABLED
			printdln(str);
#endif
		}
	}
	*/

	HIDDeviceInfo::HIDDeviceInfo() :
		_vendorID(0),
		_productID(0),
		_usagePage(0),
		_usage(0),
		handle(nullptr),
		_preparsedData(nullptr) {
	}

	HIDDeviceInfo::~HIDDeviceInfo() {
		if (_preparsedData) HidD_FreePreparsedData(_preparsedData);
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

	/*
	ByteArray HIDDeviceInfo::getRawReportDescriptor() const {
		do {
			if (!_rawReportDescriptor.isValid()) {
				_readPreparsedData();
				if (!_preparsedData) break;

				HIDP_CAPS caps;
				if (HidP_GetCaps(_preparsedData, &caps) != HIDP_STATUS_SUCCESS) break;

				hid::ReportInfo info;
				info.preparsedData = _preparsedData;
				info.usagePage = caps.UsagePage;
				info.usage = caps.Usage;
				info.numberLinkCollectionNodes = caps.NumberLinkCollectionNodes;
				info.inputData.tag = HIDReportMainItemTag::INPUT;
				info.inputData.numberButtonCaps = caps.NumberInputButtonCaps;
				info.inputData.numberValueCaps = caps.NumberInputValueCaps;
				info.outputData.tag = HIDReportMainItemTag::OUTPUT;
				info.outputData.numberButtonCaps = caps.NumberOutputButtonCaps;
				info.outputData.numberValueCaps = caps.NumberOutputValueCaps;
				info.featureData.tag = HIDReportMainItemTag::FEATURE;
				info.featureData.numberButtonCaps = caps.NumberFeatureButtonCaps;
				info.featureData.numberValueCaps = caps.NumberFeatureValueCaps;
				hid::_HIDReportDescriptor(info);

				_rawReportDescriptor = std::move(info.raw);
			}
		} while (false);

		return _rawReportDescriptor.slice(0, _rawReportDescriptor.getLength(), ByteArray::Usage::SHARED);
	}
	*/

	void* HIDDeviceInfo::getPreparsedData() const {
		if (!handle) return nullptr;
		if (!_preparsedData) HidD_GetPreparsedData(handle, &_preparsedData);
		return _preparsedData;
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
		getPreparsedData();
		if (!_preparsedData) return;

		HIDP_CAPS caps;
		if (HidP_GetCaps(_preparsedData, &caps) != HIDP_STATUS_SUCCESS) return;

		_usagePage = caps.UsagePage;
		_usage = caps.Usage;
	}


	HIDDevice::HIDDevice(HANDLE handle, PHIDP_PREPARSED_DATA preparsedData) :
		inputReportLength(0),
		outputReportLength(0),
		featureReportLength(0),
		inputBuffer(nullptr),
		outputBuffer(nullptr),
		readPending(false),
		writePending(false) {
		this->handle = handle;
		_preparsedData = preparsedData;

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

	/*
	ULONG HIDDevice::parsePressedButtons(HIDP_REPORT_TYPE reportType, USAGE usagePage, const void* reportData, ULONG reportDataLength, USAGE* outUsages, ULONG usageLength) const {
		_readPreparsedData();
		if (!_preparsedData) return 0;

		return HidP_GetUsages(reportType, usagePage, 0, outUsages, &usageLength, _preparsedData, (PCHAR)reportData, reportDataLength) == HIDP_STATUS_SUCCESS ? usageLength : 0;
	}

	std::optional<ULONG> HIDDevice::parseValue(HIDP_REPORT_TYPE reportType, USAGE usagePage, USAGE usage, const void* report, ULONG reportLength) const {
		using namespace aurora::enum_operators;

		_readPreparsedData();
		if (!_preparsedData) return std::nullopt;

		ULONG val;
		auto rst = HidP_GetUsageValue(reportType, usagePage, 0, usage, &val, _preparsedData, (PCHAR)report, reportLength);
		return HidP_GetUsageValue(reportType, usagePage, 0, usage, &val, _preparsedData, (PCHAR)report, reportLength) == HIDP_STATUS_SUCCESS ? std::make_optional(val) : std::nullopt;
	}
	*/


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

		for (int32_t i = 0; SetupDiEnumDeviceInterfaces(hDevInfo, nullptr, &guid, i, &deviceInterfaceData) != 0; ++i) {
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

	uint16_t HID::getVendorID(const HIDDevice& device) {
		return HID::getVendorID((const HIDDeviceInfo&)device);
	}

	uint16_t HID::getProductID(const HIDDeviceInfo& info) {
		if (!info.handle) return 0;

		return info.getProductID();
	}

	uint16_t HID::getProductID(const HIDDevice& device) {
		return HID::getProductID((const HIDDeviceInfo&)device);
	}

	std::wstring HID::getManufacturerString(const HIDDeviceInfo& info) {
		if (!info.handle) return std::wstring();

		WCHAR buf[256];
		if (!HidD_GetManufacturerString(info.handle, buf, sizeof(buf))) return std::wstring();
		return std::wstring(buf);
	}

	std::wstring HID::getManufacturerString(const HIDDevice& device) {
		return HID::getManufacturerString((const HIDDeviceInfo&)device);
	}

	std::wstring HID::getProductString(const HIDDeviceInfo& info) {
		if (!info.handle) return std::wstring();

		WCHAR buf[256];
		if (!HidD_GetProductString(info.handle, buf, sizeof(buf))) return std::wstring();
		return std::wstring(buf);
	}

	std::wstring HID::getProductString(const HIDDevice& device) {
		return HID::getProductString((const HIDDeviceInfo&)device);
	}

	std::string_view HID::getPath(const HIDDeviceInfo& info) {
		return info.pathView;
	}

	std::string_view HID::getPath(const HIDDevice& device) {
		return HID::getPath((const HIDDeviceInfo&)device);
	}

	uint16_t HID::getUsagePage(const HIDDeviceInfo& info) {
		return info.getUsagePage();
	}

	uint16_t HID::getUsagePage(const HIDDevice& device) {
		return HID::getUsagePage((const HIDDeviceInfo&)device);
	}

	uint16_t HID::getUsage(const HIDDeviceInfo& info) {
		return info.getUsage();
	}

	uint16_t HID::getUsage(const HIDDevice& device) {
		return HID::getUsage((const HIDDeviceInfo&)device);
	}

	/*
	ByteArray HID::getRawReportDescriptor(const HIDDeviceInfo& info) {
		return info.getRawReportDescriptor();
	}

	ByteArray HID::getRawReportDescriptor(const HIDDevice& device) {
		return HID::getRawReportDescriptor((const HIDDeviceInfo&)device);
	}
	*/

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

	void* HID::getPreparsedData(const HIDDeviceInfo& device) {
		return device.getPreparsedData();
	}

	void* HID::getPreparsedData(const HIDDevice& device) {
		return device.getPreparsedData();
	}

	/*
	size_t HID::parsePressedButtons(const HIDDevice& device, HIDReportType type, HIDUsagePage usagePage, const void* reportData, size_t reportDataLength, HIDUsage* outUsages, size_t usageLength) {
		return device.parsePressedButtons((HIDP_REPORT_TYPE)type, usagePage, reportData, reportDataLength, outUsages, usageLength);
	}

	std::optional<uint32_t> HID::parseValue(const HIDDevice& device, HIDReportType type, HIDUsagePage usagePage, HIDUsage usage, const void* reportData, size_t reportDataLength) {
		return device.parseValue((HIDP_REPORT_TYPE)type, usagePage, usage, reportData, reportDataLength);
	}
	*/
}
#endif