#include "Input.h"
#include "CreateModule.h"
#include "aurora/Debug.h"
#include "aurora/hash/xxHash.h"

//#include <hidclass.h>
//#include <winioctl.h>
#include <hidsdi.h>
#include <SetupAPI.h>

namespace aurora::modules::inputs::generic_input {
	Input::Input(Ref* loader) :
		_loader(loader),
		_context(nullptr) {
		libusb_init(&_context);
		//libusb_set_debug(_context, 3);

		hid_init();
	}

	Input::~Input() {
		hid_exit();

		if (_context) libusb_exit(_context);
	}

	events::IEventDispatcher<ModuleEvent>& Input::getEventDispatcher() {
		return _eventDispatcher;
	}

	void Input::poll() {
		if (!_context) return;

		///*
		printdln();

		auto devicesInfo = hid_enumerate(0, 0);
		for (auto devInfo = devicesInfo; devInfo; devInfo = devInfo->next) {
			if (auto dev = hid_open_path(devInfo->path); dev) {
				std::wstring wInfo;
				auto fn = [&wInfo](const std::wstring_view& data) {
					wInfo += data;
				};

				printlnTo(fn, "Device :", " vid = ", devInfo->vendor_id, " pid = ", devInfo->product_id, " [", devInfo->manufacturer_string, "]", "[", devInfo->product_string, "]");
				
				printlnTo(fn, "  path = ", devInfo->path);
				printlnTo(fn, "  hash = ", hash::xxHash::calc<64, std::endian::little>(devInfo->path, strlen(devInfo->path), 0));
				printlnTo(fn, "  sn = ", devInfo->serial_number);
				printlnTo(fn, "  interface_number = ", devInfo->interface_number);

				{
					std::string usagePageStr;

					auto usagePage = (HIDReportUsagePageType)devInfo->usage_page;
					if (usagePage >= HIDReportUsagePageType::POWER_PAGES_BEGIN && usagePage <= HIDReportUsagePageType::POWER_PAGES_END) {
						usagePageStr = "POWER_PAGES";
					} else if (usagePage >= HIDReportUsagePageType::VENDOR_DEFINED_BEGIN && usagePage <= HIDReportUsagePageType::VENDOR_DEFINED_END) {
						usagePageStr = "VENDOR_DEFINED";
					} else {
						if (auto itr = HID_REPORT_USAGE_PAGE_TYPE_MAP.find(usagePage); itr != HID_REPORT_USAGE_PAGE_TYPE_MAP.end()) {
							usagePageStr = itr->second;
						} else {
							usagePageStr = "RESERVED";
						}
					}

					printlnTo(fn, "  usage_page = ", usagePageStr);
				}

				{
					std::string usageStr;

					switch ((HIDReportUsagePageType)devInfo->usage_page) {
					case HIDReportUsagePageType::GENERIC_DESKTOP:
					{
						auto val = (HIDReportGenericDesktopPageType)devInfo->usage;
						if (auto itr = HID_REPORT_GENERIC_DISKTOP_PAGE_TYPE_MAP.find(val); itr != HID_REPORT_GENERIC_DISKTOP_PAGE_TYPE_MAP.end()) {
							usageStr = itr->second;

							if (val != HIDReportGenericDesktopPageType::KEYBOARD &&
								val != HIDReportGenericDesktopPageType::MOUSE &&
								val != HIDReportGenericDesktopPageType::GAMEPAD) continue;
						} else {
							usageStr = "RESERVED";
						}

						break;
					}
					case HIDReportUsagePageType::CONSUMER_DEVICES:
					{
						auto val = (HIDReportConsumerPageType)devInfo->usage;
						if (auto itr = HID_REPORT_CONSUMER_PAGE_TYPE_MAP.find(val); itr != HID_REPORT_CONSUMER_PAGE_TYPE_MAP.end()) {
							usageStr = itr->second;
						} else {
							usageStr = "RESERVED";
						}

						continue;

						break;
					}
					default:
					{
						usageStr = String::toString(devInfo->usage);

						continue;

						break;
					}
					}

					printlnTo(fn, "  usage = ", usageStr);
					printlnTo(fn);
				}

				printdln(wInfo);

				//uint8_t buf[256];
				//auto ret = hid_get_feature_report(dev, buf, 256);
				
				hid_close(dev);
			}
		}

		hid_free_enumeration(devicesInfo);
		//*/

		int a = 1;
		//_findDevices();
		//_findDevices2();

		//printdln("libusb Version:", LIBUSB_API_VERSION);

		
	}

	IInputDevice* Input::createDevice(const DeviceGUID& guid) {
		return nullptr;
	}

	void Input::_calcGUID(libusb_device* device, const libusb_device_descriptor& desc, DeviceGUID& guid) {
		uint32_t offset = 0;
		guid.set<false, false>(&desc.idVendor, sizeof(desc.idVendor), offset);
		offset += sizeof(desc.idVendor);
		guid.set<false, false>(&desc.idProduct, sizeof(desc.idProduct), offset);
		offset += sizeof(desc.idProduct);
		auto bus = libusb_get_bus_number(device);
		guid.set<false, false>(&bus, sizeof(bus), offset);
		offset += sizeof(bus);

		uint8_t ports[8];
		auto count = libusb_get_port_numbers(device, ports, sizeof(ports));
		if (count < 0) count = 0;

		guid.set<false, true>(ports, count, offset);
	}

	void Input::_findDevices2() {
		::GUID guid;
		HidD_GetHidGuid(&guid);

		auto hDevInfo = SetupDiGetClassDevs(&guid, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
		if (!hDevInfo) return;

		SP_DEVINFO_DATA devinfoData;
		memset(&devinfoData, 0, sizeof(devinfoData));
		devinfoData.cbSize = sizeof(SP_DEVINFO_DATA);

		SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
		deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

		char driverName[256];

		HIDD_ATTRIBUTES attributes;
		attributes.Size = sizeof(HIDD_ATTRIBUTES);

		PSP_DEVICE_INTERFACE_DETAIL_DATA detail = nullptr;
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

		printdln();

		for (int32_t i = 0; SetupDiEnumDeviceInterfaces(hDevInfo, nullptr, &guid, i, &deviceInterfaceData) != 0; ++i) {
			HANDLE handle = nullptr;
			PHIDP_PREPARSED_DATA preparsedData = nullptr;
			DWORD requiredSize = 0;

			unsigned char buf123[16] = { 18 };

			SetupDiGetDeviceInterfaceDetail(hDevInfo, &deviceInterfaceData, nullptr, 0, &requiredSize, nullptr);
			if (requiredSize == 0) goto next;

			if (mallocDetailSize < requiredSize) {
				if (detail) free(detail);

				mallocDetailSize = requiredSize;
				detail = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc(requiredSize);
				if (detail == nullptr) goto next;

				detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			}

			if (!SetupDiGetDeviceInterfaceDetail(hDevInfo, &deviceInterfaceData, detail, requiredSize, nullptr, nullptr)) goto next;

			if (!SetupDiEnumDeviceInfo(hDevInfo, i, &devinfoData)) goto next;

			if (!SetupDiGetDeviceRegistryProperty(hDevInfo, &devinfoData, SPDRP_DRIVER, nullptr, (PBYTE)driverName, sizeof(driverName), nullptr)) goto next;

			handle = CreateFile(detail->DevicePath,
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

					printdln(info, "\n");
				}
			}

		next:
			if (preparsedData) HidD_FreePreparsedData(preparsedData);
			if (handle != INVALID_HANDLE_VALUE) CloseHandle(handle);

			int a = 1;
		}

		if (detail) free(detail);
		if (linkCollectionNodes) delete[] linkCollectionNodes;
		if (buttonCaps) delete[] buttonCaps;

		SetupDiDestroyDeviceInfoList(hDevInfo);

		int a = 1;
	}

	void Input::_findDevices() {
		libusb_device** devices = nullptr;

		auto numDevs = libusb_get_device_list(_context, &devices);

		for (size_t i = 0; i < numDevs; ++i) _checkDevice(devices[i]);

		libusb_free_device_list(devices, 0);

		int a = 1;
	}

	void Input::_checkDevice(libusb_device* device) {
		using namespace std::literals;

		libusb_device_handle* handle = nullptr;
		
		libusb_device_descriptor devDesc;
		libusb_get_device_descriptor(device, &devDesc);

		if (devDesc.bDeviceClass != libusb_class_code::LIBUSB_CLASS_PER_INTERFACE || devDesc.bDeviceSubClass != libusb_class_code::LIBUSB_CLASS_PER_INTERFACE || devDesc.bDeviceSubClass != libusb_class_code::LIBUSB_CLASS_PER_INTERFACE) return;

		DeviceGUID guid(NO_INIT);
		_calcGUID(device, devDesc, guid);

		if (libusb_open(device, &handle) != LIBUSB_SUCCESS) return;

		uint8_t manufacturer[256];
		libusb_get_string_descriptor_ascii(handle, devDesc.iManufacturer, manufacturer, sizeof(manufacturer));

		uint8_t product[256];
		libusb_get_string_descriptor_ascii(handle, devDesc.iProduct, product, sizeof(product));

		std::string indent = "";

		auto isHID = false;

		printdln(indent, "Device ", ": VendorID = ", devDesc.idVendor, "  ProductID = ", devDesc.idProduct, "  [", (const char*)manufacturer, "]  [", (const char*)product, "]  NumConfs = ", devDesc.bNumConfigurations, "  bus = ", libusb_get_bus_number(device), "  Device{Cls = ", devDesc.bDeviceClass, "  SubCls = ", devDesc.bDeviceSubClass, "  Protocol = ", devDesc.bDeviceProtocol, "}");

		InternalDeviceInfo best;
		for (size_t i = 0; i < devDesc.bNumConfigurations; ++i) {
			libusb_config_descriptor* conf = nullptr;
			InternalDeviceInfo cur;

			if (libusb_get_config_descriptor(device, i, &conf) == LIBUSB_SUCCESS) {
				_checkConfiguration(handle, *conf, i, indent + "  "sv);

				libusb_free_config_descriptor(conf);
			}
		}

		if (handle) libusb_close(handle);

		printdln();
	}

	void Input::_checkConfiguration(libusb_device_handle* handle, const libusb_config_descriptor& desc, size_t index, const std::string_view& indent) {
		printdln(indent, "Configuration ", index, " : NumInterfaces = ", desc.bNumInterfaces, "  ConfValue = ", desc.bConfigurationValue, "  TotalLength = ", desc.wTotalLength);

		for (decltype(desc.bNumInterfaces) i = 0; i < desc.bNumInterfaces; ++i) {
			auto& interface = desc.interface[i];
			for (decltype(interface.num_altsetting) j = 0; j < interface.num_altsetting; ++j) _checkInterface(handle, interface.altsetting[j], j, indent + "  "sv);
		}
	}

	void Input::_checkInterface(libusb_device_handle* handle, const libusb_interface_descriptor& desc, size_t index, const std::string_view& indent) {
		if (desc.bNumEndpoints && desc.bInterfaceClass == libusb_class_code::LIBUSB_CLASS_HID) {
			printdln(indent, "Interface ", index, " : Len = ", desc.bLength, "  AlternateSetting = ", desc.bAlternateSetting, "  NumEps = ", desc.bNumEndpoints, "  Interface{Number = ", desc.bInterfaceNumber, "  Cls = ", desc.bInterfaceClass, "  SubCls = ", desc.bInterfaceSubClass, "  Protocol = ", desc.bInterfaceProtocol, "}");

			if (desc.extra_length >= 9) {
				HIDDescriptor hidDesc;
				hidDesc.set(desc.extra, desc.extra_length);

				_checkHID(handle, hidDesc, desc, indent + "  "sv);
			}

			for (decltype(desc.bNumEndpoints) i = 0; i < desc.bNumEndpoints; ++i) _checkEndpoint(handle, desc.endpoint[i], i, indent + "  "sv);
		}
	}

	void Input::_checkEndpoint(libusb_device_handle* handle, const libusb_endpoint_descriptor& desc, size_t index, const std::string_view& indent) {
		printdln(indent, "Endpoint ", index, " : Len = ", desc.bLength, "  EpAddr = ", desc.bEndpointAddress, "  Interval = ", desc.bInterval, "  Attrs = ", desc.bmAttributes, "  Refresh = ", desc.bRefresh, "  MaxPktSize = ", desc.wMaxPacketSize);
	}

	void Input::_checkHID(libusb_device_handle* handle, const HIDDescriptor& desc, const libusb_interface_descriptor& interface, const std::string_view& indent) {
		printdln(indent, "HID : Len = ", desc.bLength, "  Ver = ", desc.bcdHID, "  Country = ", desc.bCountry, "  NumDescriptors = ", desc.bNumDescriptors, "  Type = ", desc.Descriptors[0].bType, "  Size = ", desc.Descriptors[0].wLength);

		if (desc.bNumDescriptors && desc.Descriptors[0].bType == libusb_descriptor_type::LIBUSB_DT_REPORT) {
			uint8_t buf[256];
			//libusb_claim_interface(handle, interface.bInterfaceNumber);
			if (auto ret = libusb_control_transfer(handle, 0x81, LIBUSB_REQUEST_GET_DESCRIPTOR, LIBUSB_DT_REPORT << 8, interface.bInterfaceNumber, buf, sizeof(buf), 1000); ret > 0) {
				auto subIndent = indent + "  ";
				printdln(subIndent, "Report : Len = ", ret, "  Hash = ", hash::xxHash::calc<32, std::endian::native>(buf, ret, 0));

				//_checkHIDReport(buf, ret, subIndent + "  ");
			} else {
				int a = 1;
			}
			//libusb_release_interface(handle, interface.bInterfaceNumber);

			//auto ret4 = libusb_control_transfer(handle, LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_STANDARD | LIBUSB_RECIPIENT_INTERFACE, LIBUSB_REQUEST_GET_DESCRIPTOR, (LIBUSB_DT_REPORT << 8) | interface.bInterfaceNumber, 0, buf, sizeof(buf), 1000);
			int a = 1;
		}
	}

	void Input::_checkHIDReport(const void* data, size_t size, const std::string_view& indent) {
		std::string indent1(indent);
		auto isFirstNotCollectionMain = true;
		HIDReportUsagePageType usagePage = HIDReportUsagePageType::UNDEFINED;

		ByteArray ba((void*)data, size, ByteArray::Usage::SHARED);
		while (ba.getBytesAvailable()) {
			uint8_t item = ba.read<uint8_t>();
			auto size = item & 0b11;
			auto type = item >> 2 & 0b11;
			auto tag = item >> 4 & 0b1111;

			if (type == 3 && tag == 0xF) {
				size = ba.read<uint8_t>();
				tag = ba.read<uint8_t>();
			}

			auto data = ba.slice(size);
			ba.skip(size);

			std::string info = indent1 + "item("sv;

			if (auto itr = HID_REPORT_ITEM_TYPE_MAP.find((HIDReportItemType)type); itr != HID_REPORT_ITEM_TYPE_MAP.end()) {
				info += itr->second;
				info += " ";

				switch ((HIDReportItemType)type) {
				case HIDReportItemType::MAIN:
				{
					if (auto itr = HID_REPORT_MAIN_ITEM_TAG_MAP.find((HIDReportMainItemTag)tag); itr != HID_REPORT_MAIN_ITEM_TAG_MAP.end()) {
						info += itr->second;

						switch ((HIDReportMainItemTag)tag) {
						case HIDReportMainItemTag::COLLECTION:
						{
							indent1 += "  "sv;

							if (data.getBytesAvailable()) {
								auto val = data.read<uint8_t>();
								info += " ";
								if (val >= (uint16_t)HIDReportCollectionData::VENDOR_DEFINED_BEGIN && val <= (uint16_t)HIDReportCollectionData::VENDOR_DEFINED_END) {
									info += "VENDOR_DEFINED"sv;
								} else {
									if (auto itr = HID_REPORT_COLLECTION_DATA_MAP.find((HIDReportCollectionData)val); itr != HID_REPORT_COLLECTION_DATA_MAP.end()) {
										info += itr->second;
									} else {
										info += "RESERVED"sv;
									}
								}
							}

							break;
						}
						case HIDReportMainItemTag::END_COLLECTION:
						{
							auto n = isFirstNotCollectionMain ? 2 : 4;
							indent1 = indent1.substr(0, indent1.size() - n);
							info = info.substr(n);

							break;
						}
						default:
						{
							if (isFirstNotCollectionMain) {
								isFirstNotCollectionMain = false;

								indent1 += "  "sv;
							} else {
								info = info.substr(2);
							}

							if (data.getBytesAvailable()) {
								auto val = data.read<uint8_t>();

								info += " ["sv;

								info += val >> 0 & 0b1 ? "Constant "sv : "Data "sv;
								info += val >> 1 & 0b1 ? "Variable "sv : "Array "sv;
								info += val >> 2 & 0b1 ? "Relative "sv : "Absolute "sv;
								info += val >> 3 & 0b1 ? "Wrap "sv : "NoWrap "sv;
								info += val >> 4 & 0b1 ? "NonLinear "sv : "Linear "sv;
								info += val >> 5 & 0b1 ? "NoPreferred "sv : "PreferredState "sv;
								info += val >> 6 & 0b1 ? "NullState "sv : "NoNullPosition "sv;
								info += val >> 7 & 0b1 ? "Volatile"sv : "NonVolatile"sv;

								if (data.getBytesAvailable()) {
									val = data.read<uint8_t>();
									info += val >> 0 & 0b1 ? " BufferedBytes"sv : " BitField"sv;
								}

								info += "]"sv;
							}

							break;
						}
						}
					} else {
						info += String::toString(tag);
					}


					break;
				}
				case HIDReportItemType::GLOBAL:
				{
					if (auto itr = HID_REPORT_GLOBAL_ITEM_TAG_MAP.find((HIDReportGlobalItemTag)tag); itr != HID_REPORT_GLOBAL_ITEM_TAG_MAP.end()) {
						info += itr->second;

						switch ((HIDReportGlobalItemTag)tag) {
						case HIDReportGlobalItemTag::USAGE_PAGE:
						{
							if (data.getBytesAvailable()) {
								usagePage = (HIDReportUsagePageType)data.read<ba_vt::UIX>(data.getBytesAvailable());
								info += " ";

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
							}

							break;
						}
						case HIDReportGlobalItemTag::REPORT_SIZE:
						{
							info += " "sv + String::toString(data.read<ba_vt::UIX>(data.getBytesAvailable())) + "bits"sv;
							data.seekEnd();

							break;
						}
						case HIDReportGlobalItemTag::REPORT_COUNT:
						case HIDReportGlobalItemTag::LOGICAL_MINIMUM:
						case HIDReportGlobalItemTag::LOGICAL_MAXIMUM:
						{
							info += " " + String::toString(data.read<ba_vt::UIX>(data.getBytesAvailable()));
							data.seekEnd();

							break;
						}
						default:
							break;
						}
					} else {
						info += String::toString(tag);
					}

					break;
				}
				case HIDReportItemType::LOCAL:
				{
					if (auto itr = HID_REPORT_LOCAL_ITEM_TAG_MAP.find((HIDReportLocalItemTag)tag); itr != HID_REPORT_LOCAL_ITEM_TAG_MAP.end()) {
						info += itr->second;

						switch (usagePage) {
						case HIDReportUsagePageType::GENERIC_DESKTOP:
						{
							if (data.getBytesAvailable()) {
								info += " ";
								auto val = (HIDReportGenericDesktopPageType)data.read<ba_vt::UIX>(data.getBytesAvailable());
								if (auto itr = HID_REPORT_GENERIC_DISKTOP_PAGE_TYPE_MAP.find(val); itr != HID_REPORT_GENERIC_DISKTOP_PAGE_TYPE_MAP.end()) {
									info += itr->second;
								} else {
									info += "RESERVED"sv;
								}
							}

							break;
						}
						case HIDReportUsagePageType::CONSUMER_DEVICES:
						{
							if (data.getBytesAvailable()) {
								info += " ";
								auto val = (HIDReportConsumerPageType)data.read<ba_vt::UIX>(data.getBytesAvailable());
								if (auto itr = HID_REPORT_CONSUMER_PAGE_TYPE_MAP.find(val); itr != HID_REPORT_CONSUMER_PAGE_TYPE_MAP.end()) {
									info += itr->second;
								} else {
									info += "RESERVED"sv;
								}
							}

							break;
						}
						default:
							break;
						}
					} else {
						info += String::toString(tag);
					}

					break;
				}
				default:
					break;
				}
			} else {
				info += String::toString(type);
			}

			info += " "sv + String::toString(size) + ")"sv;

			if (data.getBytesAvailable()) {
				data.seekBegin();
				info += " "sv;
				do {
					info += String::toString(data.read<uint8_t>()) + " "sv;
				} while (data.getBytesAvailable());
			} else {
				info += "  ====="sv;
			}

			printdln(info);

			//printdln((type == 0 ? ""sv : (type == 1 ? "  "sv : "    "sv)), "type = ", HID_REPORT_ITEM_TYPE[type], "  tag = ", (tag < 8 || tag > 12 ? String::toString(tag) : HID_REPORT_MAIN_ITEM_TAG[tag - 8]));

			//ba.skip(size);
		}
	}
}