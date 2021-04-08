#include "CreateModule.h"
#include "Gamepad.h"
#include "aurora/HID.h"
#include "aurora/hash/xxHash.h"
#include "aurora/Debug.h"
//#include "aurora/hash/xxHash.h"

//#include <hidclass.h>
//#include <winioctl.h>
//#include <hidsdi.h>
//#include <SetupAPI.h>

namespace aurora::modules::inputs::hid_input {
	Input::Input(Ref* loader, IApplication* app, DeviceType filter) :
		_loader(loader),
		_filter(filter),
		_app(app) {
	}

	Input::~Input() {
	}

	events::IEventDispatcher<ModuleEvent>& Input::getEventDispatcher() {
		return _eventDispatcher;
	}

	void Input::poll() {
		using namespace aurora::extensions;
		using namespace aurora::enum_operators;

		if ((DeviceType::GAMEPAD & _filter) == DeviceType::UNKNOWN) return;

		std::vector<InternalDeviceInfo> newDevices;
		HID::enumDevices(&newDevices, [](const HIDDeviceInfo& info, void* custom) {
			auto newDevices = (std::vector<InternalDeviceInfo>*)custom;

			if (HID::getUsagePage(info) == 1) {
				if (HID::getUsage(info) == 5) {
					auto& dev = newDevices->emplace_back();

					auto path = HID::getPath(info);

					auto hash = hash::xxHash::calc<64, std::endian::native>(path.data(), path.size(), 0);
					dev.guid.set<false, true>(&hash, sizeof(hash), 0);
					dev.vendorID = HID::getVendorID(info);
					dev.productID = HID::getProductID(info);
					dev.type = DeviceType::GAMEPAD;
					dev.path = path;
				}
			}
		});

		std::vector<DeviceInfo> add;
		std::vector<DeviceInfo> remove;
		{
			std::scoped_lock lock(_mutex);

			for (auto& info : newDevices) {
				if (!_hasDevice(info, _devices)) add.emplace_back(info);
			}

			for (auto& info : _devices) {
				if (!_hasDevice(info, newDevices)) remove.emplace_back(info);
			}

			_devices = std::move(newDevices);
		}

		for (auto& info : remove) _eventDispatcher.dispatchEvent(this, ModuleEvent::DISCONNECTED, &info);
		for (auto& info : add) _eventDispatcher.dispatchEvent(this, ModuleEvent::CONNECTED, &info);

		/*
		using namespace aurora::enum_operators;

		::GUID guid;
		HidD_GetHidGuid(&guid);

		auto hDevInfo = SetupDiGetClassDevsW(&guid, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
		if (!hDevInfo) return;

		std::vector<InternalDeviceInfo> newDevices;

		SP_DEVINFO_DATA devinfoData;
		memset(&devinfoData, 0, sizeof(devinfoData));
		devinfoData.cbSize = sizeof(SP_DEVINFO_DATA);

		SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
		deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

		char driverName[256];

		HIDD_ATTRIBUTES attributes;
		attributes.Size = sizeof(HIDD_ATTRIBUTES);

		PSP_DEVICE_INTERFACE_DETAIL_DATA_W detail = nullptr;
		size_t mallocDetailSize = 0;

		PHIDP_LINK_COLLECTION_NODE linkCollectionNodes = nullptr;
		size_t numAllocatedLinkCollectionNodes = 0;

		PHIDP_BUTTON_CAPS buttonCaps = nullptr;
		size_t numAllocatedButtonCaps = 0;

		auto findButtonCap = [](const PHIDP_BUTTON_CAPS caps, size_t n, size_t begin, USHORT collection) {
			for (; begin < n; ++begin) {
				if (caps[begin].LinkCollection == collection) return begin;
			}

			return n + 1;
		};

		//printdln();

		for (int32_t i = 0; SetupDiEnumDeviceInterfaces(hDevInfo, nullptr, &guid, i, &deviceInterfaceData) != 0; ++i) {
			HANDLE handle = INVALID_HANDLE_VALUE;
			PHIDP_PREPARSED_DATA preparsedData = nullptr;
			DWORD requiredSize = 0;

			unsigned char buf123[16] = { 18 };

			SetupDiGetDeviceInterfaceDetailW(hDevInfo, &deviceInterfaceData, nullptr, 0, &requiredSize, nullptr);
			if (requiredSize == 0) goto next;

			if (mallocDetailSize < requiredSize) {
				if (detail) free(detail);

				mallocDetailSize = requiredSize;
				detail = (PSP_INTERFACE_DEVICE_DETAIL_DATA_W)malloc(requiredSize);
				if (detail == nullptr) goto next;

				detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			}

			if (!SetupDiGetDeviceInterfaceDetailW(hDevInfo, &deviceInterfaceData, detail, requiredSize, nullptr, nullptr)) goto next;

			if (!SetupDiEnumDeviceInfo(hDevInfo, i, &devinfoData)) goto next;

			if (!SetupDiGetDeviceRegistryPropertyW(hDevInfo, &devinfoData, SPDRP_DRIVER, nullptr, (PBYTE)driverName, sizeof(driverName), nullptr)) goto next;

			handle = CreateFileW(detail->DevicePath,
				0,
				//GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				nullptr,
				OPEN_EXISTING,
				FILE_FLAG_OVERLAPPED,
				nullptr);
			if (handle == INVALID_HANDLE_VALUE) goto next;

			if (!HidD_GetAttributes(handle, &attributes)) goto next;
			if (!HidD_GetPreparsedData(handle, &preparsedData)) goto next;

			HIDP_CAPS cap;
			size_t numButtonCaps;
			if (HidP_GetCaps(preparsedData, &cap) == HIDP_STATUS_SUCCESS) {
				{
					if (numAllocatedLinkCollectionNodes < cap.NumberLinkCollectionNodes) {
						if (linkCollectionNodes) delete[] linkCollectionNodes;

						numAllocatedLinkCollectionNodes = cap.NumberLinkCollectionNodes;
						linkCollectionNodes = new HIDP_LINK_COLLECTION_NODE[cap.NumberLinkCollectionNodes];
					}

					ULONG num = numAllocatedLinkCollectionNodes;
					HidP_GetLinkCollectionNodes(linkCollectionNodes, &num, preparsedData);
				}

				{
					numButtonCaps = cap.NumberInputButtonCaps + cap.NumberOutputButtonCaps + cap.NumberFeatureButtonCaps;
					if (numAllocatedButtonCaps < numButtonCaps) {
						if (buttonCaps) delete[] buttonCaps;

						numAllocatedButtonCaps = numButtonCaps;
						buttonCaps = new HIDP_BUTTON_CAPS[numAllocatedButtonCaps];
					}

					USHORT last = numButtonCaps;
					size_t offset = 0;
					USHORT numInputCaps = last;
					HidP_GetButtonCaps(HidP_Input, buttonCaps, &numInputCaps, preparsedData);

					last -= numInputCaps;
					offset += numInputCaps;
					USHORT numOutputCaps = last;
					HidP_GetButtonCaps(HidP_Output, buttonCaps + offset, &numOutputCaps, preparsedData);

					last -= numOutputCaps;
					offset += numOutputCaps;
					USHORT numFeatureCaps = last;
					HidP_GetButtonCaps(HidP_Feature, buttonCaps + offset, &numFeatureCaps, preparsedData);

					last -= numFeatureCaps;
					if (last != 0) {
						//error
						int a = 1;
					}
				}

				auto pd = (uint8_t*)preparsedData;//45, 64, 109, 61, 47
				for (size_t i = 0; i < 100; ++i) {
					//printdln(pd[i]);
				}

			} else {
				goto next;
			}

			{
				WCHAR buf[256];

				HidD_GetManufacturerString(handle, buf, sizeof(buf));
				std::wstring manufacturer = buf;

				HidD_GetProductString(handle, buf, sizeof(buf));
				std::wstring product = buf;

				printdln("Device :", " vid = ", attributes.VendorID, " pid = ", attributes.ProductID, " [", manufacturer, "]", " [", product, "]", " Path = ", detail->DevicePath);

				{
					std::string indent = "  ";

					std::string info;

					{
						info += indent + "UsagePage (";

						auto usagePage = (HIDReportUsagePageType)cap.UsagePage;
						if (usagePage >= HIDReportUsagePageType::POWER_PAGES_BEGIN && usagePage <= HIDReportUsagePageType::POWER_PAGES_END) {
							info += "POWER_PAGES"sv;
						} else if (usagePage >= HIDReportUsagePageType::VENDOR_DEFINED_BEGIN && usagePage <= HIDReportUsagePageType::VENDOR_DEFINED_END) {
							info += "VENDOR_DEFINED"sv;
						} else {
							if (auto itr = HID_REPORT_USAGE_PAGE_TYPE_MAP.find(usagePage); itr != HID_REPORT_USAGE_PAGE_TYPE_MAP.end()) {
								info += itr->second;
							} else {
								info += "RESERVED"sv;
							}
						}

						info += ")\n";
					}

					{
						info += indent + "Usage (";

						switch ((HIDReportUsagePageType)cap.UsagePage) {
						case HIDReportUsagePageType::GENERIC_DESKTOP:
						{
							auto val = (HIDReportGenericDesktopPageType)cap.Usage;
							if (auto itr = HID_REPORT_GENERIC_DISKTOP_PAGE_TYPE_MAP.find(val); itr != HID_REPORT_GENERIC_DISKTOP_PAGE_TYPE_MAP.end()) {
								info += itr->second;

								auto dt = DeviceType::UNKNOWN;
								switch (val) {
								case HIDReportGenericDesktopPageType::GAMEPAD:
									dt = DeviceType::GAMEPAD;
									break;
								default:
									break;
								}

								if ((dt & _filter) == DeviceType::UNKNOWN) goto next;

								if (dt != DeviceType::UNKNOWN) {
									std::wstring_view dp = detail->DevicePath;
									auto hashVal = hash::xxHash::calc<64, std::endian::little>(dp.data(), dp.size() * sizeof(std::wstring_view::value_type), 0);

									auto& info = newDevices.emplace_back();
									info.vendorID = attributes.VendorID;
									info.productID = attributes.ProductID;
									info.guid.set<false, true>(&hashVal, sizeof(hashVal));
									info.type = dt;
									info.devicePath = dp;
								}
							} else {
								info += "RESERVED";
							}

							break;
						}
						case HIDReportUsagePageType::CONSUMER_DEVICES:
						{
							auto val = (HIDReportConsumerPageType)cap.Usage;
							if (auto itr = HID_REPORT_CONSUMER_PAGE_TYPE_MAP.find(val); itr != HID_REPORT_CONSUMER_PAGE_TYPE_MAP.end()) {
								info += itr->second;
							} else {
								info += "RESERVED";
							}

							break;
						}
						default:
						{
							info += String::toString(cap.Usage);

							break;
						}
						}

						info += ")\n";
					}

					{
						if (linkCollectionNodes) {
							if (cap.NumberLinkCollectionNodes > 1) {
								int a = 1;
							}

							std::function<void(std::string&, PHIDP_LINK_COLLECTION_NODE, size_t, const std::string&)> parseCollection;
							parseCollection = [&parseCollection, &findButtonCap, buttonCaps, &cap](std::string& info, PHIDP_LINK_COLLECTION_NODE linkCollectionNodes, size_t index, const std::string& indent) {
								auto& collection = linkCollectionNodes[index];

								info += indent + "Collection (";

								auto indent1 = indent + "  ";

								auto val = (HIDReportCollectionData)collection.CollectionType;
								if (val >= HIDReportCollectionData::VENDOR_DEFINED_BEGIN && val <= HIDReportCollectionData::VENDOR_DEFINED_END) {
									info += "VENDOR_DEFINED";
								} else {
									if (auto itr = HID_REPORT_COLLECTION_DATA_MAP.find(val); itr != HID_REPORT_COLLECTION_DATA_MAP.end()) {
										info += itr->second;
									} else {
										info += "RESERVED";
									}
								}

								info += ")\n";

								size_t btnCapIndex = 0;
								size_t end = cap.NumberInputButtonCaps;
								do {
									btnCapIndex = findButtonCap(buttonCaps, end, btnCapIndex, index);
									if (btnCapIndex >= end) {
										break;
									} else {
										info += indent1 + "Input ()\n";
										++btnCapIndex;
									}
								} while (true);

								btnCapIndex = end;
								end += cap.NumberOutputButtonCaps;
								do {
									btnCapIndex = findButtonCap(buttonCaps, end, btnCapIndex, index);
									if (btnCapIndex >= end) {
										break;
									} else {
										info += indent1 + "Output ()\n";
										++btnCapIndex;
									}
								} while (true);

								btnCapIndex = end;
								end += cap.NumberFeatureButtonCaps;
								do {
									btnCapIndex = findButtonCap(buttonCaps, end, btnCapIndex, index);
									if (btnCapIndex >= end) {
										break;
									} else {
										info += indent1 + "Feature ()\n";
										++btnCapIndex;
									}
								} while (true);

								auto c = collection.FirstChild;
								while (c) {
									parseCollection(info, linkCollectionNodes, c, indent1);
									c = linkCollectionNodes[c].NextSibling;
								}

								info += indent + "EndCollection ()\n";
							};

							for (USHORT i = 0; i < cap.NumberLinkCollectionNodes; ++i) {
								if (linkCollectionNodes[i].Parent != 0) continue;

								parseCollection(info, linkCollectionNodes, i, indent);
							}
						}
					}

					//printdln(info, "\n");
				}
			}

		next:
			if (preparsedData) HidD_FreePreparsedData(preparsedData);
			if (handle != INVALID_HANDLE_VALUE) CloseHandle(handle);
		}

		if (detail) free(detail);
		if (linkCollectionNodes) delete[] linkCollectionNodes;
		if (buttonCaps) delete[] buttonCaps;

		SetupDiDestroyDeviceInfoList(hDevInfo);
		*/
	}

	IntrusivePtr<IInputDevice> Input::createDevice(const DeviceGUID& guid) {
		using namespace aurora::extensions;

		std::shared_lock lock(_mutex);

		InternalDeviceInfo* di;
		HIDDevice* hid = nullptr;
		for (auto& info : _devices) {
			if (info.guid == guid) {
				di = &info;

				switch (info.type) {
				case DeviceType::GAMEPAD:
					hid = HID::open(info.path);
					break;
				default:
					break;
				}

				break;
			}
		}

		if (!hid) return nullptr;

		IInputDevice* device = new Gamepad(*this, *di, *hid);

		return device;
	}
}